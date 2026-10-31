#pragma once

// Initialize the E-Paper power-enable pin and refresh QR when the DHCP URL changes.
void QRBegin();
void QRUpdateIfNeeded();
