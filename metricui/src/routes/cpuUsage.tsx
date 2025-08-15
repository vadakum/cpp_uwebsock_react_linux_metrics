// src/routes/cpuUsage.tsx
import { createFileRoute } from '@tanstack/react-router'
import CpuUsageComp from '../components/cpuUsageComp';

export const Route = createFileRoute('/cpuUsage')({
  component: CpuUsage,
})

function CpuUsage() {

  return (
    <>
    <CpuUsageComp />
    </>
  )
}
