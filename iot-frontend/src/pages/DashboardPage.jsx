import React, { useState } from 'react';
import DeviceList from '../components/DeviceList';
import DeviceControl from '../components/DeviceControl';

const DashboardPage = ({ token }) => {
  const [selectedDevice, setSelectedDevice] = useState(null);

  if (selectedDevice) {
    return <DeviceControl token={token} deviceId={selectedDevice} />;
  }

  return <DeviceList token={token} onSelectDevice={setSelectedDevice} />;
};

export default DashboardPage;
