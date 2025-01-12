import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import WiFiList from '../components/WiFiList';
import AddWiFiForm from '../components/AddWiFiForm';
import DeviceList from '../components/DeviceList';
import AddDeviceForm from '../components/AddDeviceForm';

const DashboardPage = ({ token }) => {
    const [showAddWiFiForm, setShowAddWiFiForm] = useState(false);
<<<<<<< HEAD
    const navigate = useNavigate(); // Use React Router's navigation hook
=======
    const [showAddDeviceForm, setShowAddDeviceForm] = useState(false);
    const [devices, setDevices] = useState([]);
>>>>>>> 24183e858e1bd87edf9d7e177e0639af58beaad9

    const toggleAddWiFiForm = () => {
        setShowAddWiFiForm((prev) => !prev);
    };

    const toggleAddDeviceForm = () => {
        setShowAddDeviceForm((prev) => !prev);
    };

    const handleAddDevice = (newDevice) => {
        setDevices((prevDevices) => [...prevDevices, newDevice]);
    };

    return (
        <div className="p-6">
            <h1 className="text-3xl font-bold mb-6">Dashboard</h1>

            {/* Add WiFi Network Toggle */}
            <button
                onClick={toggleAddWiFiForm}
                className="mb-4 bg-blue-500 text-white py-2 px-4 rounded-md hover:bg-blue-600 transition"
            >
                {showAddWiFiForm ? 'Cancel' : 'Add WiFi Network'}
            </button>
            {showAddWiFiForm && <AddWiFiForm token={token} />}
<<<<<<< HEAD

            {/* WiFi List */}
            <WiFiList token={token} />

            {/* Navigate to Bluetooth Setup */}
            <button
                onClick={() => navigate('/bluetooth-setup')}
                className="mt-6 bg-green-500 text-white py-2 px-4 rounded-md hover:bg-green-600 transition"
            >
                Setup Bluetooth
            </button>
=======
            <button
                onClick={toggleAddDeviceForm}
                className="mb-4 bg-green-500 text-white py-2 px-4 rounded-md hover:bg-green-600 transition"
            >
                {showAddDeviceForm ? 'Cancel' : 'Add Device'}
            </button>
            {showAddDeviceForm && <AddDeviceForm token={token} onAddDevice={handleAddDevice} />}
            <WiFiList token={token} />
            <DeviceList token={token} devices={devices} />
>>>>>>> 24183e858e1bd87edf9d7e177e0639af58beaad9
        </div>
    );
};

export default DashboardPage;
