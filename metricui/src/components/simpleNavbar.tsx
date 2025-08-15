import classes from './simpleNavbar.module.css'
import { AppShell, Group, ScrollArea } from '@mantine/core';
import { Link } from '@tanstack/react-router';
import { useState } from 'react';

// Icons  
import { GiCpu } from "react-icons/gi";
import { MdOutlineComputer } from "react-icons/md";
import { FaMemory } from "react-icons/fa";
import { FaHome } from "react-icons/fa";


const data = [
  { link: '/cpuUsage', label: 'CPU Usage', icon: GiCpu },
  { link: '/memUsage', label: 'Memory Usage', icon: FaMemory },
];

export function SimpleNavbar() {
  const [active, setActive] = useState('');

  const links = data.map((item, index) => (
    <Link
      to={item.link}
      className={classes.link}
      data-active={item.label === active || undefined}
      key={item.label}
      onClick={(event) => {
        setActive(item.label);
      }}
    >
      <item.icon className={classes.linkIcon} stroke={"1.5"} />
      <span>{item.label}</span>
    </Link>
  ));

  return (
    <AppShell.Navbar>
      <AppShell.Section p="md" className={classes.header}>
        <Group>
          <Link to={'/'} className={classes.link} >
          <MdOutlineComputer className={classes.linkIcon} size={25} />
          <span> Navigate Metrics </span>
          </Link>
        </Group>
      </AppShell.Section>

      <AppShell.Section grow my="md" component={ScrollArea} px="md">
        {links}
      </AppShell.Section>
      <AppShell.Section p="md" className={classes.footer}>
        <Link to={'/'} className={classes.link} >
          <FaHome className={classes.linkIcon} stroke={"1.5"} />
          <span>Home</span>
        </Link>
      </AppShell.Section>
    </AppShell.Navbar>
  );
}