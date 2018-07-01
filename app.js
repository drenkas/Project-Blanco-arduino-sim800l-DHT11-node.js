var app		= require('express')(),
	express = require('express'),
	http	= require('http').Server(app),
	io		= require('socket.io')(http),
	SerialPort	= require('serialport'),
	porta_http = 8000,
	sp = new SerialPort("/COM3", {
		baudRate: 9600,
});
const parsers = SerialPort.parsers;

// Use a `\r\n` as a line terminator
const parser = new parsers.Readline({
  delimiter: '\n'
}); //читаємо серіал-порт і компонуємо все до \n
sp.pipe(parser);
sp.on('open', () => console.log('Port open'));

parser.on('data', console.log); // тут має вивестись JSON який ми надіслали з контроллера
app.use(express.static(__dirname));
app.get('/', function (req, res) { //формуємо GET-запит
	res.sendfile('index.html');
})

parser.on('data', function(data){ 
	try {
		/* console.log("paraparparp"+JSON.parse(data)); */
		io.emit('dados', JSON.parse(data)); //надсилаємо об'єкт з даними до клієнта
		/* console.log("TTTATATATATAT" + JSON.parse(data)); */
	} catch (error) {
		/* console.log("parser poymav maslinu:"+ data + "\n"); */
	}
	

})

io.on('connection', function(socket){ //ця функція потрібна для обробки запитів від клієнта,
	//але ця версія програми цього не підтримує, тому ігноруємо її
	socket.on('Led', function (d) {
		sp.write(d);
		console.log(d);
	})

	socket.on('LedRGB', function(l){
		sp.write(l);
		console.log(l);
	})
});

sp.on('close', function(err) {console.log('Server off');});
sp.on('error', function(err) {console.error('Error', err);});
sp.on('open', function () {
	console.log('Server connect: http://localhost:' + porta_http);
});

http.listen(porta_http);