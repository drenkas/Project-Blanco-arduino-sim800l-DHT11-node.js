$(document).ready(function(){
	$("#temp").text(29); //філлери поки у нас не працює програма
	$("#humidade").text(36);

	var socket = io();
	var led; //змінні для спілкування з сервером <
	var $led = $("#led");
	var $LedRGB = $("#ledrgb"); //>
	var ctx = document.getElementById('chart').getContext('2d');//графік температури
	var ctx2 = document.getElementById('chart-2').getContext('2d');//графік вологості
	var data = { //об'єкт для створення графіка температури
	  labels: [0],
	  datasets: [{
		data: [0],
		label: 'Temperature',
		backgroundColor: '#ff6600'
	  }]
	}
	var data2 = {//об'єкт для створення графіка вологості
		labels: [0],
		datasets: [{
		  data: [0],
		  label: 'Humidity',
		  backgroundColor: '#0800ff'
		}]
	  }

	var optionsAnimations = { animation: false }
	var chart = new Chart(ctx, {//об'єкт налаштувань для  графіка температури
	  type: 'line',
	  data: data,
	  options: {
		optionsAnimations,
		scales: {
			yAxes: [{
				ticks: {
					beginAtZero:true,
					min: 10,
					max: 40    
				}
			  }]
		},
	  }
	})
	var chart2 = new Chart(ctx2, {//об'єкт налаштувань для  графіка вологості
		type: 'line',
		data: data2,
		options: {
		  optionsAnimations,
		  scales: {
			  yAxes: [{
				  ticks: {
					  beginAtZero:true,
					  min: 0,
					  max: 100    
				  }
				}]
		  },
		}
	  })
	
	socket.on('dados', function (dado) { //отримали об'єкт з сокета
		console.log("dado " + dado); //вивели в консоль, щоб переконатись що там те що нам треба
		$("#temp").text(dado.Temp); //виводимо температуру
		$("#humidade").text(dado.Humidity);//виводимо вологість
		//далі йдуть налаштування графіків, передача їм данних і рендер
		var length = data.labels.length
		if (length >= 20) {
			data.datasets[0].data.shift()
			data.labels.shift()
		}
		console.log(dado.Temp);
		data.labels.push(moment().format('HH:mm:ss'))
		data.datasets[0].data.push(dado.Temp)
		var length2 = data2.labels.length
		if (length2 >= 20) {
			data2.datasets[0].data.shift()
			data2.labels.shift()
		}
		console.log(dado.Humidity);
		data2.labels.push(moment().format('HH:mm:ss'))
		data2.datasets[0].data.push(dado.Humidity)
		chart.update()
		chart2.update()



		//то все теж для спілкування з сервером , яке не дороблене. Тому воно нам не треба (впринципі можете видалить,
	//але поки працює то не треба =) )
		led = dado.led;

		if(led == "LED01_off"){
			$led.removeClass('btn-succes').addClass('btn-danger');
			$led.attr('status', led).find('span').text("Desligado");
		} else {
			$led.removeClass('btn-danger').addClass('btn-succes');
			$led.attr('status', led).find('span').text("Ligado");
		}

		$led.click(function () {
			status = (led == "LED01_off")?"LED01_on":"LED01_off";
			socket.emit('Led', status);
			console.log(status);
		})
	})
	$LedRGB.ColorPicker({
		color:'red',
		onShow: function (colpkr) {
			$(colpkr).fadeIn(500);
			return false;
		},
		onHide: function (colpkr) {
			$(colpkr).fadeOut(500);
			return false;
		},
		onChange: function (hab, hex, rgb) {
			$LedRGB.css('backgroundColor', '#' + hex);
			stringRGB = JSON.stringify(rgb);
			console.log(stringRGB);
			socket.emit('LedRGB', stringRGB);
		}
	});
})