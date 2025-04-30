function scan() {
    fetch('/scan')
      .then(response => response.text())
      .then(data => { console.log(data); location.reload(); });
  }

  function spectrum() {
    fetch('/spectrum')
      .then(response => response.json())
      .then(data => drawSpectrumChart(data))
      .catch(error => console.error('Error fetching spectrum:', error));
  }

  function stop() {
    fetch('/stop')
      .then(response => response.text())
      .then(data => { console.log(data); location.reload(); });
  }

  function drawSpectrumChart(data) {
    const ctx = document.getElementById('spectrumChart').getContext('2d');
    const labels = Object.keys(data).map(ch => "CH " + ch);
    const values = Object.values(data);

    if (window.spectrumChart) {
      window.spectrumChart.data.labels = labels;
      window.spectrumChart.data.datasets[0].data = values;
      window.spectrumChart.update();
      return;
    }

    window.spectrumChart = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: labels,
        datasets: [{
          label: 'RSSI (dBm)',
          data: values,
          backgroundColor: '#8cff00'
        }]
      },
      options: {
        responsive: true,
        scales: {
          y: {
            beginAtZero: false,
            reverse: true,
            min: -100,
            max: -10,
            title: {
              display: true,
              text: 'Signal Strength (dBm)'
            }
          }
        }
      }
    });
  }