// script.js

document.addEventListener("DOMContentLoaded", () => {
  const toggleBtn = document.getElementById('toggleBtn');
  const startFlashBtn = document.getElementById('startFlashBtn');
  const stopFlashBtn = document.getElementById('stopFlashBtn');
  const onDurationInput = document.getElementById('onDuration');
  const offDurationInput = document.getElementById('offDuration');
  const statusText = document.getElementById('status');
  const digitalLed = document.getElementById('digitalLed');

  let flashingInterval = null;
  let isVirtualFlashing = false;

  // Update digital LED indicator based on state
  function updateDigitalLed(state) {
    digitalLed.classList.remove('led-on', 'led-off');
    
    if (state === "ON") {
      digitalLed.classList.add('led-on');
    } else if (state === "OFF") {
      digitalLed.classList.add('led-off');
    }
  }

  // Toggle LED
  toggleBtn.addEventListener('click', () => {
    fetch('/toggle', {
      method: 'POST'
    })
      .then(response => {
        if (!response.ok) {
          throw new Error(`Toggle request failed: ${response.statusText}`);
        }
        return response.json();
      })
      .then(data => {
        if (data.status === "ON" || data.status === "OFF") {
          statusText.innerText = data.status;
          updateDigitalLed(data.status);
          // Stop any ongoing flashing when toggling
          stopVirtualFlashing();
        }
      })
      .catch(err => {
        console.error("Error toggling LED:", err);
        //alert("Failed to toggle LED.");
      });
  });

  // Start Flashing
  startFlashBtn.addEventListener('click', () => {
    const onDuration = parseInt(onDurationInput.value);
    const offDuration = parseInt(offDurationInput.value);

    // Validate input durations
    if (isNaN(onDuration) || isNaN(offDuration) || onDuration <= 0 || offDuration <= 0) {
      alert("Please enter valid positive numbers for durations.");
      return;
    }

    // Create JSON payload
    const payload = {
      onDuration: onDuration,
      offDuration: offDuration
    };

    fetch('/startFlash', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(payload)
    })
      .then(response => {
        if (!response.ok) {
          throw new Error(`Start Flash request failed: ${response.statusText}`);
        }
        return response.json();
      })
      .then(data => {
        if (data.status === "Flashing") {
          statusText.innerText = "Flashing";
          startVirtualFlashing(onDuration, offDuration);
        }
      })
      .catch(err => {
        console.error("Error starting flash:", err);
        alert("Failed to start flashing.");
      });
  });

  // Stop Flashing
  stopFlashBtn.addEventListener('click', () => {
    fetch('/stopFlash', {
      method: 'POST'
    })
      .then(response => {
        if (!response.ok) {
          throw new Error(`Stop Flash request failed: ${response.statusText}`);
        }
        return response.json();
      })
      .then(data => {
        if (data.status === "Flashing Stopped" || data.status === "OFF") {
          statusText.innerText = "OFF";
          updateDigitalLed("OFF");
          stopVirtualFlashing();
        }
      })
      .catch(err => {
        console.error("Error stopping flash:", err);
        alert("Failed to stop flashing.");
      });
  });

  // Start Virtual Flashing
  function startVirtualFlashing(onDuration, offDuration) {
    if (isVirtualFlashing) {
      clearTimeout(flashingTimeout);
    }
    isVirtualFlashing = true;

    function flash(isOn) {
      if (!isVirtualFlashing) return;

      if (isOn) {
        statusText.innerText = "ON";
        updateDigitalLed("ON");
        flashingTimeout = setTimeout(() => flash(false), onDuration);
      } else {
        statusText.innerText = "OFF";
        updateDigitalLed("OFF");
        flashingTimeout = setTimeout(() => flash(true), offDuration);
      }
    }

    flash(true); // Start with ON
  }

  // Stop Virtual Flashing
  function stopVirtualFlashing() {
    isVirtualFlashing = false;
    if (flashingTimeout) {
      clearTimeout(flashingTimeout);
      flashingTimeout = null;
    }
  }
});
