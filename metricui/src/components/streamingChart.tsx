import React, { useEffect, useRef, useState } from "react";
import ReactECharts from 'echarts-for-react';


/**
 * Represents the data structure for a time-series chart.
 * @property ts - An array of timestamp strings for the x-axis.
 * @property series - A map where each key is a series name (e.g., "cpu", "mem")
 *                    and the value is an array of corresponding data points.
 */
export interface TsSeries {
  ts: string[],
  series: Record<string, number[]>;
}

interface Props {
  /** The time-series data to render in the chart. */
  data?: TsSeries;
  children?: React.ReactNode;
}

/**
 * A reusable React component that renders a streaming time-series chart using Apache ECharts.
 * It dynamically generates chart options based on the provided data prop.
 */
const StreamingChart: React.FC<Props> = ({ data, children }) => {

  //console.log("StreamingChart data:", data);

  let options = {};
  // Only generate chart options if there is valid data with at least one series.
  if (data && Object.keys(data.series).length > 0) {

    const palette = [
      '#00ff00', // green for overall
      '#00bfff', '#ffb300', '#ff4081', '#7c4dff', '#00e676',
      '#ff1744', '#ffd600', '#64dd17', '#0091ea'
    ];

    // The legend is only shown if there are multiple series to display.
    const keys = Object.keys(data.series);
    const showLegend = keys.length > 1;

    options = {
      title: undefined,
      backgroundColor: '#1e1e1e',
      tooltip: { trigger: "axis" },
      legend: {
        data: keys,
        show: showLegend,
        textStyle: { color: '#ccc' },
        selectedMode: 'multiple', // allow toggling series visibility
      },
      xAxis: {
        type: "category",
        data: data?.ts,
        axisLine: { lineStyle: { color: '#888' } },
        axisLabel: { color: '#888', show: false },
        splitLine: { show: true, lineStyle: { color: '#ccc', width: 0.2 } },
      },
      yAxis: {
        type: "value",
        min: 0,
        max: 100,
        axisLine: { lineStyle: { color: '#888' } },
        axisLabel: { color: '#888', show: true },
        splitLine: { show: true, lineStyle: { color: '#ccc', width: 0.2 } },
      },
      series: keys.map((seriesName, idx) => ({
        name: seriesName,
        type: "line",
        data: data.series[seriesName],
        showSymbol: false,
        smooth: false,
        lineStyle: {
          color: palette[idx % palette.length],
          width: 1,
        },
      })),
    };
  }


  return (
    <div style={{ width: "100%", height: "350px" }}>
      <ReactECharts option={options} style={{ height: '100%' }} />
    </div>);
};

export default StreamingChart;
