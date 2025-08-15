// src/routes/cpuUsage.tsx
import { createFileRoute } from '@tanstack/react-router'
import MemUsageComp from '../components/memUsageComp'

export const Route = createFileRoute('/memUsage')({
  component: CpuUsage,
})

function CpuUsage() {

  return (
    <>
    <MemUsageComp />
    </>
  )
}
