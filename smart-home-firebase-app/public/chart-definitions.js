// Create the charts when the web page loads
window.addEventListener('load', onload);

function onload(event) {
  chartT = createTemperatureChart();
}

// Create Temperature Chart
function createTemperatureChart() {
  var chart = new Highcharts.Chart({
    colors: ['#2b908f', '#90ee7e', '#f45b5b', '#7798BF', '#aaeeee', '#ff0066',
      '#eeaaee', '#55BF3B', '#DF5353', '#7798BF', '#aaeeee'],
    chart: {
      renderTo: 'chart-temperature',
      type: 'spline',
      backgroundColor: {
        linearGradient: { x1: 0, y1: 0, x2: 1, y2: 1 },
        stops: [
          [0, '#424242'],
          [1, '#262626']
        ]
      },
    },
    series: [
      {
        name: 'BME280',
        style: {
          color: '#E0E0E3'
        }
      }
    ],
    title: {
      text: undefined
    },
    plotOptions: {
      line: {
        animation: false,
        dataLabels: {
          enabled: true
        }
      },
      series: {
        color: '#FF7F50',
        marker: {
          enabled: true,
        }
      }
    },
    xAxis: {
      type: 'datetime',
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: {
        text: 'Hőmérséklet (°C)',
        style: {
          color: '#E0E0E3'
        }
      },
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
    },
    credits: {
      enabled: false
    }
  });
  return chart;
}

// Create Pressure Chart
function createPressureChart() {
  var chart = new Highcharts.Chart({
    colors: ['#2b908f', '#90ee7e', '#f45b5b', '#7798BF', '#aaeeee', '#ff0066',
      '#eeaaee', '#55BF3B', '#DF5353', '#7798BF', '#aaeeee'],
    chart: {
      renderTo: 'chart-pressure',
      type: 'spline',
      backgroundColor: {
        linearGradient: { x1: 0, y1: 0, x2: 1, y2: 1 },
        stops: [
          [0, '#424242'],
          [1, '#262626']
        ]
      },
    },
    series: [{
      name: 'BME280',
      labels: {
        style: {
          color: '#E0E0E3'
        }
      }
    }],
    title: {
      text: undefined
    },
    plotOptions: {
      line: {
        animation: false,
        dataLabels: {
          enabled: true
        }
      },
      series: {
        color: '#6495ED',
        marker: {
          enabled: true,
        }
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' },
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
    },
    yAxis: {
      title: {
        text: 'Nyomás (hPa)',
        style: {
          color: '#E0E0E3'
        }
      },
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
    },
    credits: {
      enabled: false
    }
  });
  return chart;
}

// Create HUmidity Chart
function createHumidityChart() {
  var chart = new Highcharts.Chart({
    colors: ['#2b908f', '#90ee7e', '#f45b5b', '#7798BF', '#aaeeee', '#ff0066',
      '#eeaaee', '#55BF3B', '#DF5353', '#7798BF', '#aaeeee'],
    chart: {
      renderTo: 'chart-humidity',
      type: 'spline',
      backgroundColor: {
        linearGradient: { x1: 0, y1: 0, x2: 1, y2: 1 },
        stops: [
          [0, '#424242'],
          [1, '#262626']
        ]
      },
    },
    series: [{
      name: 'BME280',
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
    }],
    title: {
      text: undefined
    },
    plotOptions: {
      line: {
        animation: false,
        dataLabels: {
          enabled: true
        }
      },
      series: {
        color: '#40E0D0',
        marker: {
          enabled: true,
        }
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' },
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
    },
    yAxis: {
      title: {
        text: 'Páratartalom (%)',
        style: {
          color: '#E0E0E3'
        }
      },
      labels: {
        style: {
          color: '#E0E0E3'
        }
      },
    },
    credits: {
      enabled: false
    }
  });
  return chart;
}