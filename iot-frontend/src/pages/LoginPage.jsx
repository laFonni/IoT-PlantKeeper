import React, { useState } from 'react';
import LoginForm from '../components/LoginForm';
import DashboardPage from './DashboardPage';

const LoginPage = () => {
  const [token, setToken] = useState('');

  if (token) {
    return <DashboardPage token={token} />;
  }

  return <LoginForm onLogin={setToken} />;
};

export default LoginPage;
