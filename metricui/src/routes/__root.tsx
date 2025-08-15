// src/routes/__root.tsx
/// <reference types="vite/client" />
import '@mantine/core/styles.css'
import { MantineProvider, AppShell, Burger, Group } from '@mantine/core'
import { useDisclosure } from '@mantine/hooks';
import type { ReactNode } from 'react'
import {
  Outlet,
  createRootRoute,
  HeadContent,
  Scripts,
} from '@tanstack/react-router'

import { SimpleNavbar } from '../components/simpleNavbar'

export const Route = createRootRoute({
  head: () => ({
    meta: [
      {
        charSet: 'utf-8',
      },
      {
        name: 'viewport',
        content: 'width=device-width, initial-scale=1',
      },
      {
        title: 'Metric UI',
      },
    ],
  }),
  component: RootComponent,
})

function RootComponent() {
  return (
    <RootDocument>
      <Outlet />
    </RootDocument>
  )
}

function RootDocument({ children }: Readonly<{ children: ReactNode }>) {
  const [opened, { toggle }] = useDisclosure(false);

  return (
    <html>
      <head>
        <HeadContent />
      </head>
      <body>
        <MantineProvider defaultColorScheme='dark'>
          <AppShell
            header={{ height: 60 }}
            navbar={{ width: 300, breakpoint: 'sm' }}
            padding="md"
          >

            <AppShell.Header>
              <Group h="100%" px="md">
                <Burger opened={opened} onClick={toggle} hiddenFrom="sm" size="sm" />
                Metric UI Header Content...
              </Group>
            </AppShell.Header>

            <AppShell.Navbar p="md">
              <SimpleNavbar />
            </AppShell.Navbar>

            <AppShell.Main>
              {children}
            </AppShell.Main>

          </AppShell>

        </MantineProvider>

        <Scripts />
      </body>
    </html>
  )
}