import { Box, Divider, Group } from '@mantine/core';
import React from 'react';
import useWebSocket, { ReadyState } from "react-use-websocket";
import StreamingChart, { TsSeries } from './streamingChart';
import cloneDeep from 'lodash.clonedeep';
import appConfig from '../appConfig.json';
import ConnectionHeader from './connectionHeader';

const SocketUrl = appConfig.WebsockUrl;
const MaxStreamPoints = appConfig.MaxStreamPoints;
const DefaultRec: TsSeries = { ts: [], series: {/*cpu: [], cpu1 : [], cpu1 :[],...*/ } };

const CpuUsageComp = () => {

  const { sendMessage, lastMessage, readyState } = useWebSocket(SocketUrl, {
    shouldReconnect: () => true,
    share: true,
    onMessage: (event) => { handleMessage(event.data); },
  });

  const [records, setRecords] = React.useState<TsSeries>(cloneDeep(DefaultRec));
  const [responseMessage, setResponseMessage] = React.useState<string>("");

  /*
   * Callback to handle new messages
   * Expected messages:
   * 1. Response to subscription
   * {
   *  "type" : "sub_response", "status":"<success|failure>",
   *  "message" : "<some message from server>"
   * }
   * 2. Streaming subscription
   * {
   *  "type" : "sub_stream", "metric_name":"cpu_usage", "ts":<unix epoch ts>, 
   *  "data":[{"name":"cpu","usage":13},{"name":"cpu0","usage":15},...]
   * }
   * 
   * We will transform this into a record structure and pass on to chart
   * component:
   * { ts: [], "series": {"cpu" : [13, 40,...], "cpu0" : [15, 25, ...]}};
   * 
   */
  const handleMessage = React.useCallback((newMessage) => {
    let inMsg = JSON.parse(newMessage);
    if (inMsg.type === "sub_response" || inMsg.type === "error_response") {
      setResponseMessage(() => {
        return inMsg.status + " | [" + inMsg.message + "]";
      });
    }
    else if (inMsg.type === "sub_stream" && inMsg.metric_name === "cpu_usage") {
      let cpuNames = inMsg.data.map((kv: any) => kv.name);
      setRecords((prevRecs) => {
        let newRecs: TsSeries = cloneDeep(DefaultRec); // We dont want to hold reference to DefaultRec              
        // Fill series        
        cpuNames.forEach((name: string, idx: number) => {
          let usage = inMsg.data[idx].usage;
          if (name in prevRecs.series) {
            newRecs.series[name] = [...prevRecs.series[name], usage].slice(-MaxStreamPoints);
          } else {
            newRecs.series[name] = [usage]
          }
        });
        // Fill ts
        newRecs.ts = [...prevRecs.ts, new Date(inMsg.ts * 1000).toLocaleTimeString()].slice(-MaxStreamPoints);
        return newRecs;
      });
    }
  }, []);


  React.useEffect(() => {
    if (readyState === ReadyState.OPEN) {
      sendMessage(JSON.stringify({
        "metric_name": "cpu_usage",
        "type": "sub"
      }));
    }
  }, [readyState, sendMessage]);


  // Split overall and per-core data
  let overallCpu = cloneDeep(DefaultRec);
  overallCpu.ts = records.ts;
  overallCpu.series["cpu"] = records.series?.cpu ?? [];

  let perCore = cloneDeep(DefaultRec);
  perCore.ts = records.ts;
  perCore.series = cloneDeep(records.series);
  if ("cpu" in perCore.series) {
    delete perCore.series.cpu; // Remove overall CPU from per-core data
  }

  return (
    <>
      <ConnectionHeader readyState={readyState} responseMessage={responseMessage} />

      {/* {<p>Last Message: {lastMessage ? lastMessage.data : 'No messages yet'}</p>} */}

      <Box style={{ display: 'flex', gap: '10px' }}>
        <Box style={{ width: '50%' }}>
          <h3>Overall CPU Usage</h3>
          <StreamingChart data={overallCpu} />
        </Box>
        <Box style={{ width: '50%' }}>
          <h3>Per-Core CPU Usage</h3>
          <StreamingChart data={perCore} />
        </Box>
      </Box>
    </>
  );
};

export default CpuUsageComp;
