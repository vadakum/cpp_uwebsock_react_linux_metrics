import { Box, Group } from '@mantine/core';
import React from 'react';
import useWebSocket, { ReadyState } from "react-use-websocket";
import StreamingChart, { TsSeries } from './streamingChart';
import cloneDeep from 'lodash.clonedeep';
import appConfig from '../appConfig.json';
import ConnectionHeader from './connectionHeader';

const SocketUrl = appConfig.WebsockUrl;
const MaxStreamPoints = appConfig.MaxStreamPoints;
const DefaultRec: TsSeries = { ts: [], series: { 'mem': [] } };

const MemUsageComp = () => {

  const { sendMessage, lastMessage, readyState } = useWebSocket(SocketUrl, {
    shouldReconnect: () => true,
    onMessage: (event) => {
      handleMessage(event.data);
    },
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
   *  "type" : "sub_stream", "metric_name":"mem_usage", "ts":<unix epoch ts>, 
   *  "data":{"usage":13}
   * }
   * 
   * We will transform this into a record structure and pass on to chart
   * component:
   * { ts: [], "series": {"mem" : [13, 40,...]}};
   * 
   */
  const handleMessage = React.useCallback((newMessage) => {
    let inMsg = JSON.parse(newMessage);
    if (inMsg.type === "sub_response" || inMsg.type === "error_response") {
      setResponseMessage(() => {
        return inMsg.status + " | [" + inMsg.message + "]";
      });
    }
    else if (inMsg.type === "sub_stream" && inMsg.metric_name === "mem_usage") {
      setRecords((prevRecs) => {
        let newRecs: TsSeries = cloneDeep(DefaultRec); // We dont want to hold reference to DefaultRec
        newRecs.series.mem = [...prevRecs.series.mem, inMsg.data.usage].slice(-MaxStreamPoints);
        newRecs.ts = [...prevRecs.ts, new Date(inMsg.ts * 1000).toLocaleTimeString()].slice(-MaxStreamPoints);
        return newRecs;
      });
    }
  }, []);

  React.useEffect(() => {
    if (readyState === ReadyState.OPEN) {
      sendMessage(JSON.stringify({
        "metric_name": "mem_usage",
        "type": "sub"
      }));
    }
  }, [readyState, sendMessage]);

  return (
    <>
      <ConnectionHeader readyState={readyState} responseMessage={responseMessage} />

      <Box style={{ width: '50%' }}>
        <h3>Memory Usage</h3>
        <StreamingChart data={records} />
      </Box>
    </>
  );
};

export default MemUsageComp;
