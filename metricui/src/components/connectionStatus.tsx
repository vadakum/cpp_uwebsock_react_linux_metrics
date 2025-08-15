import React from 'react';
import { ReadyState } from 'react-use-websocket';
import { ColorSwatch, Group } from '@mantine/core';

interface ConnectionStatusProps {
  readyState: ReadyState;
}

const ConnectionStatus: React.FC<ConnectionStatusProps> = ({ readyState }) => {
  const statusInfo = {
    [ReadyState.CONNECTING]: { color: 'yellow', text: 'Connecting...' },
    [ReadyState.OPEN]: { color: 'lightgreen', text: 'Connected' },
    [ReadyState.CLOSING]: { color: 'yellow', text: 'Closing...' },
    [ReadyState.CLOSED]: { color: 'red', text: 'Disconnected' },
    [ReadyState.UNINSTANTIATED]: { color: 'red', text: 'Not Ready' },
  };

  const { color, text } = statusInfo[readyState];

  return (
    <Group style={{ display: 'flex', alignItems: 'center' }}>
    <ColorSwatch color={color} size={15} title={text} style={{ marginRight: "-10px" }}/>
      <span><i>{text}</i></span>
    </Group>

  );
};

export default ConnectionStatus;
