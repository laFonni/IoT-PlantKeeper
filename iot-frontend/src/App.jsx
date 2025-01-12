import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import LoginPage from './pages/LoginPage';
import RegisterPage from './pages/RegisterPage';
import DashboardPage from './pages/DashboardPage';
import WiFiPage from './pages/WiFiPage';
import ProtectedRoute from './ProtectedRoute';
import AuthProvider from './AuthContext';
import BluetoothSetup from './components/BluetoothSetup';

const App = () => (
    <AuthProvider>
        <Router>
            <Routes>
                <Route path="/login" element={<LoginPage />} />
                <Route path="/register" element={<RegisterPage />} />
                <Route
                    path="/dashboard"
                    element={
                        <ProtectedRoute>
                            <DashboardPage />
                        </ProtectedRoute>
                    }
                />
                <Route
                    path="/wifi"
                    element={
                        <ProtectedRoute>
                            <WiFiPage />
                        </ProtectedRoute>
                    }
                />
                <Route
                    path="/bluetooth-setup"
                    element={
                        <ProtectedRoute>
                            <BluetoothSetup />
                        </ProtectedRoute>
                    }
                />
            </Routes>
        </Router>
    </AuthProvider>
);

export default App;
