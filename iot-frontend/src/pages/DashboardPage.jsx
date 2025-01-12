import React, { useState } from 'react';
import WiFiList from '../components/WiFiList';
import AddWiFiForm from '../components/AddWiFiForm';
import DeviceList from '../components/DeviceList';
import AddDeviceForm from '../components/AddDeviceForm';

const DashboardPage = () => {
    const [showAddWiFiForm, setShowAddWiFiForm] = useState(false);
    const [showAddDeviceForm, setShowAddDeviceForm] = useState(false);

    const toggleAddWiFiForm = () => {
        setShowAddWiFiForm((prev) => !prev);
    };

    const toggleAddDeviceForm = () => {
        setShowAddDeviceForm((prev) => !prev);
    };

    return (
        <div className="p-6 bg-background min-h-screen">
          <h1 className="text-3xl font-bold mb-6 text-primary">Dashboard</h1>
          <button
            onClick={toggleAddWiFiForm}
            className="mb-4 bg-primary text-white py-2 px-4 rounded-md hover:bg-secondary transition"
          >
            {showAddWiFiForm ? 'Cancel' : 'Add WiFi Network'}
          </button>
          {showAddWiFiForm && <AddWiFiForm />}
          <button
            onClick={toggleAddDeviceForm}
            className="mb-4 bg-secondary text-white py-2 px-4 rounded-md hover:bg-primary transition"
          >
            {showAddDeviceForm ? 'Cancel' : 'Add Device'}
          </button>
          {showAddDeviceForm && <AddDeviceForm />}
          <WiFiList />
          <DeviceList />
        </div>
      );
};

export default DashboardPage;
