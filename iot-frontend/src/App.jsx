import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import LoginPage from './pages/LoginPage';
import RegisterPage from './pages/RegisterPage';
import WiFiPage from './pages/WiFiPage';
import DashboardPage from './pages/DashboardPage';
import AuthProvider from './AuthContext';
import ProtectedRoute from './ProtectedRoute';

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
      </Routes>
    </Router>
  </AuthProvider>
);

export default App;
