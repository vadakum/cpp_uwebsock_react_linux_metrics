import React from 'react';
import { ReadyState } from 'react-use-websocket';
import { Divider, Group } from '@mantine/core';
import ConnectionStatus from './connectionStatus';
import appConfig from '../appConfig.json';

interface ConnectionHeaderProps {
  readyState: ReadyState;
  responseMessage : string
}

const ConnectionHeader: React.FC<ConnectionHeaderProps> = ({ readyState, responseMessage }) => {
  return (
      <Group justify='space-evenly' style={{gap: '10px', border : '1px solid gray', padding: '2px'}}>
        <h4> WebSocket: </h4>
        <span style={{ color: 'GrayText' }}>{appConfig.WebsockUrl}</span>
        <Divider orientation='vertical'></Divider>
        <h4>Connection Status: </h4>
        <ConnectionStatus readyState={readyState} />
        <Divider orientation='vertical'></Divider>
        <h4>Subscription status: </h4>
        <span>{responseMessage}</span>
      </Group>

  );
};

export default ConnectionHeader;
