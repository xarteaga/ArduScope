/**
 * Created by xavier on 8/03/14.
 */
var numValues = 8192;
var wsUri = "ws://192.168.10.2/";
var output;
var data = [];
var counter = 0;
var stamp;
var samplingTime=0;
var alpha = 0.5;
function init() {
    output = document.getElementById("output");
    testWebSocket();
}
function testWebSocket() {
    websocket = new WebSocket(wsUri);

    websocket.onopen = function (evt) {
        onOpen(evt)
    };
    websocket.onclose = function (evt) {
        onClose(evt)
    };
    websocket.onmessage = function (evt) {
        onMessage(evt);
    };
    websocket.onerror = function (evt) {
        onError(evt)
    };
}
function onOpen(evt) {
    writeToScreen("CONNECTED");
    doSend("WebSocket rocks");
}
function onClose(evt) {
    writeToScreen("DISCONNECTED");
}

function onMessage(evt) {
    //var d = JSON.parse(evt.data.substr(0, 31));
    var res = evt.data.split(" ");
    if (typeof (data[counter])==='undefined')
        data[counter] = {x: counter, y : parseInt(res[1])};
    else
        data[counter].y = parseInt(res[1]);

    var pstamp = stamp;
    stamp = parseInt(res[0]);
    if (pstamp<stamp){
        samplingTime = samplingTime*(1-alpha) + alpha*(stamp-pstamp);
    }

    counter = (counter + 1) % numValues;

    //writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data + '</span>');
}
function onError(evt) {
    writeToScreen('<span style="color: #ff0019;">ERROR:</span> ' + evt.data);
}
function doSend(message) {
    writeToScreen("SENT: " + message);
    websocket.send(message);
}
function writeToScreen(message) {
    var pre = document.createElement("p");
    pre.style.wordWrap = "break-word";
    pre.innerHTML = message;
    output.appendChild(pre);
}
window.addEventListener("load", init, false);


var margin = {top: 20, right: 20, bottom: 30, left: 50},
    width = 960 - margin.left - margin.right,
    height = 500 - margin.top - margin.bottom;

//var svg = d3.select("graph").append("svg:svg").attr("width", "100%").attr("height", "100%");

var svg = d3.select("#graph").append("svg")
    .attr("width", "100%")
    .attr("height", "100%")
    .append("g")
    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
/**/

var x = d3.scale.linear()
    .range([0, width]).domain([0, numValues]);

var y = d3.scale.linear()
    .range([height, 0]).domain([0, 1023]);

svg.append("path")
    .data([data])
    .attr("class", "line")
    .attr("d", line);

var xAxis = d3.svg.axis()
    .scale(x)
    .orient("bottom");

var yAxis = d3.svg.axis()
    .scale(y)
    .orient("left");

var line = d3.svg.line()
    .x(function (d) {
        return x(d.x);
    })
    .y(function (d) {
        return y(d.y);
    }).interpolate("linear");

for (i = 0; i < numValues-10; i++) {
    data[i] = {x: i, y: 0};
}

svg.append("g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + height + ")")
    .call(xAxis);

svg.append("g")
    .attr("class", "y axis")
    .call(yAxis)
    .append("text")
    .attr("transform", "rotate(-90)")
    .attr("y", 6)
    .attr("dy", ".71em")
    .style("text-anchor", "end")
    .text("Raw value (10 bit)");

//svg.append("svg:path").attr("d", line(data));
var first = true;

function redraw() {
     svg.selectAll("path")
     .data([data]) // set the new data
         .attr("transform", "translate(" + x(1) + ")") // set the transform to the right by x(1) pixels (6 for the scale we've set) to hide the new value
     .attr("d", line) // apply the new data values ... but the new value is hidden at this point off the right of the canvas
     .transition() // start a transition to bring the new value into view
     .ease("linear")
     .duration(0.01) // for this demo we want a continual slide so set this to the same as the setInterval amount below
     .attr("transform", "translate(" + x(0) + ")"); // animate a slide to the left back to x(0) pixels to reveal the new value*/
}

redraw();
var timer = setInterval(function () {
        document.getElementById("fs").innerHTML = "The sampling frequency is " + Math.round(1000000/samplingTime)/1000 + " kHz";
        redraw();
    }, 200
);
