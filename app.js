"use strict";

// C library API
const ffi = require("ffi-napi");

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require("express-fileupload");

app.use(fileUpload());
app.use(express.static(path.join(__dirname + "/uploads")));

// Minimization
const fs = require("fs");
const JavaScriptObfuscator = require("javascript-obfuscator");

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get("/", function (req, res) {
	res.sendFile(path.join(__dirname + "/public/index.html"));
});

// Send Style, do not change
app.get("/style.css", function (req, res) {
	//Feel free to change the contents of style.css to prettify your Web app
	res.sendFile(path.join(__dirname + "/public/style.css"));
});

// Send obfuscated JS, do not change
app.get("/index.js", function (req, res) {
	fs.readFile(
		path.join(__dirname + "/public/index.js"),
		"utf8",
		function (err, contents) {
			const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
				compact: true,
				controlFlowFlattening: true,
			});
			res.contentType("application/javascript");
			res.send(minimizedContents._obfuscatedCode);
		}
	);
});

//Respond to POST requests that upload files to uploads/ directory
app.post("/upload", function (req, res) {
	if (!req.files) {
		return res.status(400).send("No files were uploaded.");
	}

	let uploadFile = req.files.uploadFile;

	// Use the mv() method to place the file somewhere on your server
	uploadFile.mv("uploads/" + uploadFile.name, function (err) {
		if (err) {
			return res.status(500).send(err);
		}

		res.redirect("/");
	});
});

//Respond to GET requests for files in the uploads/ directory
app.get("/uploads/:name", function (req, res) {
	fs.stat("uploads/" + req.params.name, function (err, stat) {
		if (err == null) {
			res.sendFile(path.join(__dirname + "/uploads/" + req.params.name));
		} else {
			console.log("Error in file downloading route: " + err);
			res.send("");
		}
	});
});

//******************** Your code goes here ********************

//Sample endpoint
app.get("/endpoint1", function (req, res) {
	let retStr = req.query.stuff + " " + req.query.junk;
	res.send({
		stuff: retStr,
	});
});

app.listen(portNum);
console.log("Running app at localhost: " + portNum);

// Creating an object called sharedLib that will contain the C functions
let sharedLib = ffi.Library("./libgpxparser", {
	GPXFiletoJSON: ["string", ["string"]],
	GPXFiletoRouteListJSON: ["string", ["string"]],
	GPXFiletoTrackListJSON: ["string", ["string"]],
	GPXFiletoRouteGPXDataListJSON: ["string", ["string"]],
	GPXFiletoTrackGPXDataListJSON: ["string", ["string"]],
	renameGPXComponent: ["int", ["string", "string", "string", "int"]],
	createGPXFile: ["int", ["string", "string"]],
	addRouteToFile: ["int", ["string", "string"]],
	addWaypointToRoute: ["int", ["string", "string"]],
	routeListOfRoutesBetween: [
		"string",
		["string", "float", "float", "float", "float", "float"],
	],
	trackListOfRoutesBetween: [
		"string",
		["string", "float", "float", "float", "float", "float"],
	],
	numberOfRoutesWithLengthFromFile: ["int", ["string", "float", "float"]],
	numberOfTracksWithLengthFromFile: ["int", ["string", "float", "float"]],
});

// Respond to get request to get the attributes of GPX file, sending an array of GPX attributes attached with their respective file names
app.get("/GPXattributes", function (req, res) {
	// Reads all the files in the uploads directory
	let files = fs.readdirSync("uploads");
	let JSONObjectArray = [];
	let failedUploads = [];

	// Traverses through each file name concatenating the "uploads/" directory so the file could be opened by apps.js
	files.forEach((file) => {
		let filePath = "uploads/" + file;

		// Calls on the C function to take in the file name and getting the GPX attributes as a JSON string then parsing it as an object and adding it onto the array of objects
		// Also adds the file name of the current GPX file onto the JSON object as a key value pair
		let parsedJSON;
		let JSONString = sharedLib.GPXFiletoJSON(filePath);
		if (JSONString !== "{}") {
			parsedJSON = JSON.parse(JSONString.replace("\n", "\\n"));
			parsedJSON.fileName = file;
			JSONObjectArray.push(parsedJSON);
			// Else the file was invalid and is removed from uploads with an error message and appends it to the array of failed files
		} else {
			console.log(`File failed to upload and was removed: ${file}`);
			fs.unlinkSync(filePath);
			failedUploads.push(file);
		}
	});

	// Sending the array of JSON strings and array of files that failed to validate
	console.log("Sending data from /GPXattributes endpoint");
	res.send({
		JSONObjectArray: JSONObjectArray,
		failedUploads: failedUploads,
	});
});

// Responds to get request to get the component data of a speecific file, sending an array containing routes and tracks objects in JSON format for each file
app.get("/componentData", function (req, res) {
	// Reads all the files in the uploads directory
	let files = fs.readdirSync("uploads");
	let JSONFileArrayRoutes = [];
	let JSONFileArrayTracks = [];

	// Traverses through each file name concatenating the "uploads/" directory so the file could be opened by apps.js
	files.forEach((file) => {
		let filePath = "uploads/" + file;

		// Calls on the C function to take in the file name and getting array of JSON strings containing routes for that specific file and parsing it into an object
		let JSONArray = sharedLib.GPXFiletoRouteListJSON(filePath);
		let parsedJSONArray = JSON.parse(JSONArray.replace("\n", "\\n"));

		// Traverses through each JSON string and adding the respective file name as a key value pair
		parsedJSONArray.forEach((JSONString) => {
			JSONString.fileName = file;
		});

		// Adds the array of routes for the current file to the array containing all the file routes
		JSONFileArrayRoutes.push(parsedJSONArray);
	});

	// Traverses through each file nam concatenating the "uploads/" directory sot he file could be opened by apps.js
	files.forEach((file) => {
		let filePath = "uploads/" + file;

		// Calls on the C funciton to take in the file name and getting array of JSON strings containing tracks for that specific file and parsing it into an object
		let JSONArray = sharedLib.GPXFiletoTrackListJSON(filePath);
		let parsedJSONArray = JSON.parse(JSONArray.replace("\n", "\\n"));

		// Traverses through each JSON string and adding the respective file name as a key value pair
		parsedJSONArray.forEach((JSONString) => {
			JSONString.fileName = file;
		});

		// Adds the array of tracks for the current file to the array containing all the file tracks
		JSONFileArrayTracks.push(parsedJSONArray);
	});

	// Sending the array of an array of JSON strings for the routes and tracks of each respective file
	console.log("Sending data from /componentData endpoint");
	res.send({
		routeData: JSONFileArrayRoutes,
		trackData: JSONFileArrayTracks,
	});
});

// Responds to a get request to get the other data of a specific component of a specific file
app.get("/otherData", function (req, res) {
	// Variable to hold the array of JSON strings containing other data for the specific component specified by the user
	let otherDataArray = [];

	// If the component includes "Route" means that the user chose to view a routes other data
	if (req.query.componentChosen.includes("Route")) {
		// Gets the array of another array containing objects for all the routes in the file currently being viewed by the user (each index of outer array holds an array containing each other data point at every index)
		let routesOtherDataArray = sharedLib.GPXFiletoRouteGPXDataListJSON(
			`uploads/${req.query.fileName}`
		);

		// Gets the specific route the user wants to view for other data
		let routeNumber = req.query.componentChosen[6];

		// Parsing the routesOtherDataArray to create an array that holds data for each route at each index and "\n" is a special character to JSON.parse so must escape with "\\n"
		routesOtherDataArray = routesOtherDataArray.replace("\n", "\\n");
		let routesOtherData = JSON.parse(routesOtherDataArray);

		// Getting the array of JSON strings representing for the route chosen by the user and setting otherDataArray equal to it
		otherDataArray = routesOtherData[routeNumber - 1];

		// If the component includes "Track" means that the user chose to view a tracks other data
	} else if (req.query.componentChosen.includes("Track")) {
		// Gets the array of another array containing objects for all the tracks in the file currently being viewed by the user
		let tracksOtherDataArray = sharedLib.GPXFiletoTrackGPXDataListJSON(
			`uploads/${req.query.fileName}`
		);
		// Gets the specific track the user wants to view for other data
		let trackNumber = req.query.componentChosen[6];

		// Parsing the tracksOtherDataArray to create an array that holds data for each track at each index and "\n" is a special character to JSON.parse so must escape with "\\n"
		tracksOtherDataArray = tracksOtherDataArray.replace("\n", "\\n");
		let tracksOtherData = JSON.parse(tracksOtherDataArray);

		// Getting the array of JSON strings representing other data for the track chosen by the user setting otherDataArray equal to it
		otherDataArray = tracksOtherData[trackNumber - 1];
	}

	// Sending the array of JSON strings holding the other data for the component specified by the user in the file specified by the user
	console.log("Sending data from /OtherData endpoint");
	res.send(otherDataArray);
});

// Responds to a get request to rename a specific component specified by the user
app.get("/Rename", function (req, res) {
	// Calls on the renameGPXComponent to rename the component specified by the user
	let returnValue = sharedLib.renameGPXComponent(
		`uploads/${req.query.fileName}`,
		req.query.newName,
		req.query.componentType,
		req.query.componentNumber
	);

	// Checks the returnValue of renameGPXComponent
	let status = "";
	// If the returnValue is 1, stores "SUCCESS" in the JSON string meaning the rename succeeded
	if (returnValue == 1) {
		status = `{"status":"SUCCESS"}`;
	}
	// If the returnValue is 0, stores "FAIL" in the JSON string meaning the rename failed
	else {
		status = `{"status":"FAIL"}`;
	}
	// Parses the JSON string and sends it back
	status = JSON.parse(status);
	console.log("Sending data from /Rename endpoint");
	res.send(status);
});

// Makes it so that any form data is parsed as form data is sent through the body of the post request for "/createGPX"
app.use(express.urlencoded({ extended: true }));
// Responds to the post request, creating the GPX file with the values specified by the user and sending back the status
app.post("/createGPX", function (req, res) {
	// Creates a new object to store the version and creator entered by the user
	let fileObject = { version: 1.1, creator: req.body.creator };

	// Creates a new GPX file with the inputs from the user writing it to the uploads directory
	let returnValue = sharedLib.createGPXFile(
		JSON.stringify(fileObject),
		"uploads/" + req.body.fileName
	);

	// If returnValue is 1, the writing succeeded and responds with success
	if (returnValue === 1) {
		console.log(
			"Responding to post request to create a new GPX file, SUCCESS\n"
		);
		res.send("SUCCESS");
	}
	// Else, the writing failed and responds with failure
	else {
		console.log("Responding to post request to create a new GPX file, FAIL\n");
		res.send("FAIL");
	}
});

// Responds to post request, adding the route to the GPX file specified with values specified by the user and sending back the status
app.post("/routeCreate", function (req, res) {
	// Adds the route specified by the user to the file at the end
	let returnValue = sharedLib.addRouteToFile(
		"uploads/" + req.body.fileName,
		req.body.routeName
	);

	// If returnValue is 0, means adding route failed and sends FAIL
	if (returnValue === 0) {
		console.log(
			"Responding to post request to add a route to the specified GPX file, FAIL"
		);
	}
	// If adding the route to the GPX file succeeded, responds with SUCCESS
	else {
		console.log(
			"Responding to post request to add a route to the specified GPX file, SUCCESS"
		);
		res.send("SUCCESS");
	}
});

// Responds to post request, adding waypoints to the route previously added to the specified GPX file and sends back the status
app.post("/addWaypoint", function (req, res) {
	// Adds the waypoints entered by the user to the file at the last route which is the route that was added above
	req.body.waypoint.forEach((waypoints) => {
		let returnValue = sharedLib.addWaypointToRoute(
			"uploads/" + req.body.fileName,
			JSON.stringify(waypoints)
		);
		// If the returnValue for adding a waypoint to the route is 0, means adding the waypoint to the route failed and sends FAIL
		if (returnValue === 0) {
			console.log(
				"Responding to post request to add a waypoint to the route in the specified GPX file, FAIL"
			);
			res.send("FAIL");
		}
	});

	// If all the waypoints added to the route properly, sends SUCCESS
	console.log(
		"Responding to post request to add a waypoint to the route in the specified GPX file, SUCCESS"
	);
	res.send("SUCCESS");
});

// Responds to get request, getting all the routes and tracks between the source and dest latitude/longitude entered by the user
app.get("/findPath", function (req, res) {
	// Stores all the file names inside the "uploads" directory
	let files = fs.readdirSync("uploads");

	// Variables to hold the routes and tracks between the points
	let routeListArray = [];
	let trackListArray = [];

	// Traverses through all the files
	files.forEach((file) => {
		// Gets the list of routes that are between the source and dest within the comparison accuracy
		let routeList = sharedLib.routeListOfRoutesBetween(
			"uploads/" + file,
			req.query.sourceLat,
			req.query.sourceLon,
			req.query.destLat,
			req.query.destLon,
			req.query.delta
		);

		// Gets the list of tracks that are between the source and dest within the comparison accuracy
		let trackList = sharedLib.trackListOfRoutesBetween(
			"uploads/" + file,
			req.query.sourceLat,
			req.query.sourceLon,
			req.query.destLat,
			req.query.destLon,
			req.query.delta
		);

		// If both list are not empty, adds it to the respective route or track list array
		if (routeList !== "[]") {
			routeListArray.push(routeList);
		}
		if (trackList !== "[]") {
			trackListArray.push(trackList);
		}
	});

	// Sends a successful response message to the server console and sends the array of routes and tracks containing JSON strings containing routes and tracks between the user inputs
	console.log(
		"Responding to get request to get all routes and tracks between the source and dest entered by the user, SUCCESS"
	);
	// Sends the two arrays containing routes and tracks between the two points given by the user
	res.send({
		routeList: routeListArray,
		trackList: trackListArray,
	});
});

// Responds to get request, getting the number of routes and tracks with the length inputted by the user with a delta of 10m
app.get("/numberOfRoutesAndTracksWithLen", function (req, res) {
	// Stores all the file names inside the "uploads" directory
	let files = fs.readdirSync("uploads");

	// Variables to store the number of routes and tracks with the specified length
	let numRoutes = 0;
	let numTracks = 0;

	// Traverses through all the files in the uploads directory
	files.forEach((file) => {
		// If the routeLen doesn't equal 0, that means the user inputted a length to search for in the routes
		if (req.query.routeLen != 0) {
			numRoutes += sharedLib.numberOfRoutesWithLengthFromFile(
				"uploads/" + file,
				req.query.routeLen,
				10
			);
		}
		// If the trackLen doesn't equal 0, that means the user inputted a length to search for in the tracks
		if (req.query.trackLen != 0) {
			numTracks += sharedLib.numberOfTracksWithLengthFromFile(
				"uploads/" + file,
				req.query.routeLen,
				10
			);
		}
	});

	// Sends the number of routes/tracks found in the files with the length specified by the user as JSON strings
	console.log(
		"Responding to get request to get number of routes and tracks with specified length by the user, SUCCESS"
	);
	res.send({
		numberRoutes: JSON.stringify(numRoutes),
		numberTracks: JSON.stringify(numTracks),
	});
});
