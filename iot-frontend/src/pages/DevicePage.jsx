import React, { useState, useEffect, useContext } from 'react';
import { useParams } from 'react-router-dom';
import { Line } from 'react-chartjs-2';
import axios from 'axios';
import { AuthContext } from '../AuthContext';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
} from 'chart.js';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

const DevicePage = () => {
  const { deviceId } = useParams();
  const { token } = useContext(AuthContext);
  const [sensorData, setSensorData] = useState({
    soilMoisture: [],
    temperature: [],
    lightLevel: [],
    waterPump: [],
    lamp: []
  });

  useEffect(() => {
    const fetchSensorData = async () => {
      try {
        const response = await axios.get(`http://localhost:4000/telemetry/${deviceId}`, {
          headers: { Authorization: `Bearer ${token}` }
        });
        const data = response.data;

        const soilMoisture = data.filter(d => d.sensorType === 'soilMoisture').map(d => ({ x: d.timestamp, y: d.value }));
        const temperature = data.filter(d => d.sensorType === 'temperature').map(d => ({ x: d.timestamp, y: d.value }));
        const lightLevel = data.filter(d => d.sensorType === 'lightLevel').map(d => ({ x: d.timestamp, y: d.value }));
        const waterPump = data.filter(d => d.sensorType === 'waterPump').map(d => ({ x: d.timestamp, y: d.value }));
        const lamp = data.filter(d => d.sensorType === 'lamp').map(d => ({ x: d.timestamp, y: d.value }));

        setSensorData({ soilMoisture, temperature, lightLevel, waterPump, lamp });
      } catch (err) {
        console.error('Failed to fetch sensor data:', err);
      }
    };

    fetchSensorData();
  }, [deviceId, token]);

  const chartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: {
        position: 'top',
      },
    },
    scales: {
      y: {
        beginAtZero: true,
      },
    },
  };

  const charts = [
    {
      title: 'Soil Moisture',
      data: {
        labels: sensorData.soilMoisture.map(d => d.x),
        datasets: [{
          label: 'Soil Moisture (%)',
          data: sensorData.soilMoisture.map(d => d.y),
          borderColor: 'rgb(53, 162, 235)',
          backgroundColor: 'rgba(53, 162, 235, 0.5)',
        }]
      }
    },
    {
      title: 'Temperature',
      data: {
        labels: sensorData.temperature.map(d => d.x),
        datasets: [{
          label: 'Temperature (°C)',
          data: sensorData.temperature.map(d => d.y),
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.5)',
        }]
      }
    },
    {
      title: 'Light Level',
      data: {
        labels: sensorData.lightLevel.map(d => d.x),
        datasets: [{
          label: 'Light Level (lux)',
          data: sensorData.lightLevel.map(d => d.y),
          borderColor: 'rgb(255, 205, 86)',
          backgroundColor: 'rgba(255, 205, 86, 0.5)',
        }]
      }
    },
    {
      title: 'Water Pump',
      data: {
        labels: sensorData.waterPump.map(d => d.x),
        datasets: [{
          label: 'Water Pump State',
          data: sensorData.waterPump.map(d => d.y),
          borderColor: 'rgb(75, 192, 192)',
          backgroundColor: 'rgba(75, 192, 192, 0.5)',
        }]
      }
    },
    {
      title: 'Lamp',
      data: {
        labels: sensorData.lamp.map(d => d.x),
        datasets: [{
          label: 'Lamp State',
          data: sensorData.lamp.map(d => d.y),
          borderColor: 'rgb(153, 102, 255)',
          backgroundColor: 'rgba(153, 102, 255, 0.5)',
        }]
      }
    }
  ];

  return (
    <div className="p-6 max-w-6xl mx-auto bg-background">
      <h1 className="text-3xl font-bold mb-6 text-primary">Device {deviceId} Sensors</h1>
      <div className="grid gap-6 md:grid-cols-1 lg:grid-cols-2">
        {charts.map((chart, index) => (
          <div key={index} className="bg-white p-4 rounded-lg shadow-lg border border-secondary">
            <h2 className="text-xl font-semibold mb-4 text-secondary">{chart.title}</h2>
            <div className="h-96">
              <Line options={chartOptions} data={chart.data} />
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};

export default DevicePage;
