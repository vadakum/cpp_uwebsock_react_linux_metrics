// src/routes/index.tsx
import { createFileRoute } from '@tanstack/react-router'

export const Route = createFileRoute('/')({
  component: Home,
})

function Home() {
  return (
    <>
      <h1>Welcome to the Metric UI</h1>
      <p>Click on the CPU Usage or Memory Usage section (Navbar item)</p>
    </>
  )
}