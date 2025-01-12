import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import LoginPage from './pages/LoginPage';
import RegisterPage from './pages/RegisterPage';
import DashboardPage from './pages/DashboardPage';
import WiFiPage from './pages/WiFiPage';
import ProtectedRoute from './ProtectedRoute';
import AuthProvider from './AuthContext';
import DevicePage from './pages/DevicePage';
import Navbar from './components/Navbar';

const Layout = ({ children }) => (
  <div className="min-h-screen bg-background">
    <Navbar />
    <div className="container mx-auto px-4 py-8">
      {children}
    </div>
  </div>
);

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
              <Layout>
                <DashboardPage />
              </Layout>
            </ProtectedRoute>
          }
        />
        <Route
          path="/wifi"
          element={
            <ProtectedRoute>
              <Layout>
                <WiFiPage />
              </Layout>
            </ProtectedRoute>
          }
        />
        <Route
          path="/device/:deviceId"
          element={
            <ProtectedRoute>
              <Layout>
                <DevicePage />
              </Layout>
            </ProtectedRoute>
          }
        />
      </Routes>
    </Router>
  </AuthProvider>
);

export default App;
