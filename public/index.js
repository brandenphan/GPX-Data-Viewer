// Put all onload AJAX calls here, and event listeners
$(document).ready(function () {
	// On page-load AJAX Example
	$.ajax({
		type: "get", //Request type
		dataType: "json", //Data type - we will use JSON for almost everything
		url: "/endpoint1", //The server endpoint we are connecting to
		data: {
			stuff: "Value 1",
			junk: "Value 2",
		},
		success: function (data) {
			/*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
			$("#blah").html(
				"On page load, received string '" + data.stuff + "' from server"
			);
			//We write the object to the console to show that the request was successful
			console.log(data);
		},
		fail: function (error) {
			// Non-200 return, do something with error
			$("#blah").html("On page load, received error from server");
			console.log(error);
		},
	});

	// Event listener form example , we can use this instead explicitly listening for events
	// No redirects if possible
	$("#someform").submit(function (e) {
		$("#blah").html("Form has data: " + $("#entryBox").val());
		e.preventDefault();
		//Pass data to the Ajax call, so it gets passed to the server
		$.ajax({
			//Create an object for connecting to another waypoint
		});
	});

	// On-load, loads all GPX attribute information regarding the successfully loaded files and appends their information onto the first table
	$.ajax({
		// Get request for a json data type from /GPXattributes
		type: "get",
		dataType: "json",
		url: "/GPXattributes",
		// Function if the get request is successful
		success: function (GPXData) {
			// Traverses through the array of objects, GPXData contains only valid GPX files as only valid GPX files were included on the server side
			$.each(GPXData.JSONObjectArray, function (i, data) {
				// Appends the GPX file data onto the first table giving various alert messages to the alert box and the console
				console.log(`${data.fileName} successful upload`);
				$("#FileLogRows").append(`<tr>
                <th scope="row"><a href="../uploads/${data.fileName}" download ">${data.fileName}</a></th>
                <th>${data.version}</th>
                <th>${data.creator}</th>
                <th>${data.numWaypoints}</th>
                <th>${data.numRoutes}</th>
                <th>${data.numTracks}</th>
                </tr>`);
				$("#statusContainer").append(`<p>File loaded: ${data.fileName}</p>`);

				// Appends the GPX file name onto the file drop down menu options
				$("#fileDropdownMenu").append(
					`<button class="dropdown-item" onclick="dropDownFunction(this)" type="button">${data.fileName}</button>`
				);
			});

			// Checking the size of the array of objects, if it is 0, means that there are no files on the server and displays no file for the table
			if (GPXData.JSONObjectArray.length === 0) {
				$("#FileLogRows").append(`<tr>
                <th scope="row">No files</th>
                </tr>`);
				$("#statusContainer").append(`<p>No files to load</p>`);
			}

			// If the failed uploads is greater than 0, alerts the user, puts it in the alert box and prints it to the console the files that failed to upload
			if (GPXData.failedUploads != 0) {
				console.log(
					`Files failed to upload and was removed: ${GPXData.failedUploads}`
				);
				$("#statusContainer").append(
					`<p>Files failed to upload and was removed: ${GPXData.failedUploads}</p>`
				);
				alert(
					`Files failed to upload and was removed: ${GPXData.failedUploads}`
				);
			}
		},
		// Function if the get request is unsuccessful for any reason, appends the error message onto the alert box and sends an error message to the console
		fail: function (error) {
			$("#statusContainer").append(
				`<p>Error with onload AJAX call for GPX file attributes</p>`
			);
			console.log(
				"Error with onload AJAX call when loading file GPX attributes"
			);
		},
	});

	// Hiding the FindPathTable and header upon loading the website
	$("#FindPathTableHeader").hide();
	$("#FindPathTable").hide();
});

// Function to load the file table without the on-load
function loadFileTable() {
	// Ajax get request to fill the file log table
	$.ajax({
		// Get request for a json data type from /GPXattributes
		type: "get",
		dataType: "json",
		url: "/GPXattributes",
		// Function if the get request is successful
		success: function (GPXData) {
			// Removing the rows off the table so that it can be reappended later and the file drop down
			$("#FileLogRows tr").remove();
			$("#fileDropdownMenu button").remove();

			// Traverses through the array of objects, GPXData contains only valid GPX files as only valid GPX files were included on the server side
			$.each(GPXData.JSONObjectArray, function (i, data) {
				// Appends the GPX file data onto the first table giving various alert messages to the alert box
				$("#FileLogRows").append(`<tr>
                <th scope="row"><a href="../uploads/${data.fileName}" download ">${data.fileName}</a></th>
                <th>${data.version}</th>
                <th>${data.creator}</th>
                <th>${data.numWaypoints}</th>
                <th>${data.numRoutes}</th>
                <th>${data.numTracks}</th>
                </tr>`);

				// Appends the GPX file name onto the file drop down menu options
				$("#fileDropdownMenu").append(
					`<button class="dropdown-item" onclick="dropDownFunction(this)" type="button">${data.fileName}</button>`
				);
			});

			// If the failed uploads is greater than 0, alerts the user, puts it in the alert box and prints it to the console the files that failed to upload
			if (GPXData.failedUploads != 0) {
				console.log(
					`Files failed to upload and was removed: ${GPXData.failedUploads}`
				);
				$("#statusContainer").append(
					`<p>Files failed to upload and was removed: ${GPXData.failedUploads}</p>`
				);
				alert(
					`Files failed to upload and was removed: ${GPXData.failedUploads}`
				);
			}
		},
		// Function if the get request is unsuccessful for any reason, appends the error message onto the alert box and sends an error message to the console
		fail: function (error) {
			$("#statusContainer").append(
				`<p>Error with AJAX call for GPX file attributes</p>`
			);
			console.log("Error with AJAX call when loading file GPX attributes");
		},
	});
}

// Function to clear any input fields upon the user pressing the cancel button on the modal
function cancelButton() {
	$("#nameBox").val("");
	$("#addCreatorForm").val("");
	$("#addFileNameForm").val("");
	$("#newRouteName").val("");
	$("#latitudeForm").val("");
	$("#longitudeForm").val("");
	$("#sourceLongitudeForm").val("");
	$("#sourceLatitudeForm").val("");
	$("#destLongitudeForm").val("");
	$("#destLatitudeForm").val("");
	$("#deltaForm").val("");
	$("#numberOfRoutesLength").val("");
	$("#numberOfTracksLength").val("");
}

// Hold the file currently being viewed by the user and the file button to later get the innerHTML value of which ultimately holds the file name
let currentFile = "";
let currentFileButton = "";

// Function when a user presses a button to select a file, appends data to the view panel table and various modals
function dropDownFunction(fileName) {
	// Resetting the alert box and console to tell the user that they have selected this file
	$("#statusContainer p").remove();
	$("#statusContainer").append(
		`<p>Opening file for viewing: ${fileName.innerHTML}</p>`
	);
	console.log(`Opening file for viewing: ${fileName.innerHTML}`);

	// Saving the button in the currentFileButton variable
	currentFileButton = fileName;

	// Setting the current file to be viewed variable getting the innerHTML of the button the user clicked on
	currentFile = fileName.innerHTML;

	// Making a get request for the route and track data of the user selected file
	$.ajax({
		// Get request for a JSON data type from /RouteListData
		type: "get",
		data: "json",
		url: "/componentData",
		// Function if the get request is successful
		success: function (componentData) {
			// Removing the contents of the table upon the user choosing a file and also removing the buttons in the "Show other Data" and "Rename" modal
			$("#ViewPanelRows tr").remove();
			$("#ContainerForOtherDataButtons button").remove();
			$("#ContainerForRenameComponentButtons button").remove();

			// Traverses through the array of arrays containing JSON strings of routes
			$.each(componentData.routeData, function (i, data) {
				// If the array isn't empty, traverses through the array
				if (data !== "[]") {
					// Traversing through the JSON strings in the array
					$.each(data, function (i, JSONString) {
						// If the JSON string matches the respective file chosen by the user, appends the file routes to the table
						if (JSONString.fileName === fileName.innerHTML) {
							$("#ViewPanelRows").append(`<tr>
							<th scope="row">Route ${i + 1}</th>
							<th>${JSONString.name}</th>
							<th>${JSONString.numPoints}</th>
							<th>${JSONString.len}</th>
							<th>${JSONString.loop}</th>
							</tr>`);

							// Appends the components as buttons onto the modal that shows when the "Show other data" and "Rename" button is pressed
							$("#ContainerForOtherDataButtons").append(`<button
								type="button"
								id="ComponentButton"
								class="btn btn-primary Button"
								onclick="showData(this)"
								data-dismiss="modal"
							>Route ${i + 1}</button>`);
							$("#ContainerForRenameComponentButtons").append(`<button
							type="button"
							id="RenameComponentButton"
							class="btn btn-primary Button"
							onclick="renameData(this)"
							data-dismiss="modal"
							data-toggle="modal"
							data-target="#RenameForm"
							>Route ${i + 1}</button>`);
						}
					});
				}
			});

			// Travereses through the array of arrays containing JSON strings of tracks
			$.each(componentData.trackData, function (i, data) {
				// If the array isn't empty, traverses through the array
				if (data !== "[]") {
					// Traversing through the JSON strings in the array
					$.each(data, function (i, JSONString) {
						// If the JSON string matches the respective file chosen by the user, appends the file tracks to the table
						if (JSONString.fileName === fileName.innerHTML) {
							$("#ViewPanelRows").append(`<tr>
							<th scope="row">Track ${i + 1}</th>
							<th>${JSONString.name}</th>
							<th>${JSONString.numPoints}</th>
							<th>${JSONString.len}</th>
							<th>${JSONString.loop}</th>
							</tr>`);

							// Appends the components as buttons onto the modal that shows when the "Show other data" and "Rename button is pressed
							$("#ContainerForOtherDataButtons").append(`<button
								type="button"
								id="ComponentButton"
								class="btn btn-primary Button"
								onclick="showData(this)"
								data-dismiss="modal"
							>Track ${i + 1}</button>`);
							$("#ContainerForRenameComponentButtons").append(`<button
							type="button"
							id="RenameComponentButton"
							class="btn btn-primary Button"
							onclick="renameData(this)"
							data-dismiss="modal"
							data-toggle="modal"
							data-target="#RenameForm"
							>Track ${i + 1}</button>`);
						}
					});
				}
			});

			// If there were no components for the file, puts it in the table
			if ($("#ViewPanelRows tr").length === 0) {
				$("#ViewPanelRows").append(`<tr>
				<th scope="row">This file has no components</th>
				</tr>`);
			}
		},
		// Function if the get request is unsuccessful for any reason, appends the error message onto the alert box and sends an error message to the console
		fail: function (error) {
			$("#statusContainer").append(
				`<p>Error with get request for components for a GPX file</p>`
			);
			console.log("Error with get request for components for a GPX file");
		},
	});
}

// Error check to send an error message to the user that they must choose a file before trying to press "Show Other Data"
$("#ShowOtherData").on("click", function () {
	// If the currentFile is empty, means a file has not been opened yet
	if (currentFile === "") {
		// Adding error messages to the show other data modal
		$("#ContainerForOtherDataButtons p").remove();
		$("#ContainerForOtherDataButtons").append(
			`<p>Please choose a file before trying to show other data</p>`
		);
		// Adding error messages to the alert box and console
		$("#statusContainer p").remove();
		$("#statusContainer").append(
			`<p>Error: trying to press "Show Other Data" button without choosing a file to view first</p>`
		);
		console.log(
			`Error: trying to press "Show Other Data" button without choosing a file to view first`
		);
		// Else, a file has been opened and removes any previous error messages put on the modal
	} else {
		$("#ContainerForOtherDataButtons p").remove();
	}
});

// Error check to send an error message to the user that they must choose a file before trying to press "Rename"
$("#Rename").on("click", function () {
	// If the currentFile is empty, means a file has not been opened yet
	if (currentFile === "") {
		// Adding error messages to show rename modal
		$("#ContainerForRenameComponentButtons p").remove();
		$("#ContainerForRenameComponentButtons").append(
			`<p>Please choose a file before trying to rename</p>`
		);
		// Adding error messages to the alert box and console
		$("#statusContainer p").remove();
		$("#statusContainer").append(
			`<p>Error: trying to press "Rename" button without choosing a file to view first</p>`
		);
		console.log(
			`Error: trying to press "Rename" button without choosing a file to view first`
		);
	}
	// Else, a file has been opened and removes any previous error messages put on the modal
	else {
		$("#ContainerForRenameComponentButtons p").remove();
	}
});

// Function for when a user selects a button to look at the other data for a component
function showData(component) {
	// Appends to the alert box and prints to console that the user is looking at a specific component
	$("#statusContainer").append(
		`<p>Looking at component: ${component.innerHTML} of the file: ${currentFile}</p>`
	);
	console.log(
		`Retrieving component: ${component.innerHTML} of the file: ${currentFile} successful`
	);

	// Making a get request for the other data of the component specified by the user
	$.ajax({
		type: "get",
		dataType: "json",
		url: "/otherData",
		data: {
			fileName: currentFile, // Sends the currentFile chosen by the user to the get request so the server knows which file to open
			componentChosen: component.innerHTML, // Sends the component chosen by the user so the get request knows which specific otherData object to return
		},
		// Function if the the get request succeeds
		success: function (componentOtherData) {
			// If the component has no other data, alerts that special message to the user
			if (componentOtherData.length === 0) {
				alert(
					`Other data for component: ${component.innerHTML}\n\n No other data!`
				);
			}
			// If the component has other data, gets the other data alerting the user
			else {
				// String that holds all the key value pairs of all the JSON strings found in the componentOtherData object
				let otherDataString = `Other data for component: ${component.innerHTML}\n\n`;

				// Traversing through the object containing JSON strings and getting key value pairs, storing it with nice formatting within the result string
				$.each(componentOtherData, function (i, data) {
					// Traversing through the JSON string and getting all the key value pairs in the JSON string appending it onto result with formatting
					for ([key, value] of Object.entries(data)) {
						otherDataString += `${key}: ${value}\n`;
					}
				});

				// Sends an alert containing the other data for the component chosen
				alert(otherDataString);
			}
		},
		// Functions if the get request fails for any reason
		fail: function (error) {
			$("#statusContainer").append(
				`<p>Error with get request for other data of a component</p>`
			);
			console.log("Error with get request for other data of a component");
		},
	});
}

// Variables to hold the component information chosen by the user
let componentType = "";
let componentNumber;

// Function to set the componenetType and componentNumber upon the user choosing a specific component to rename
function renameData(component) {
	// If the innerHTML has "Route" means that the component type is a route and sets it
	if (component.innerHTML.includes("Route")) {
		componentType = "Route";
	}
	// Else the innerHTML contains "Track" and sets the component type equal to "Track"
	else {
		componentType = "Track";
	}

	// Gets the component number and stores it in componentNumber for example "Route 3" has a component number of 3
	componentNumber = component.innerHTML[6];
}

// Upon clicking the rename submit done, renames the component they specified and sends appropriate messages
$("#RenameSubmitButton").on("click", function () {
	// Sends a get request with various queries to rename a specific component specified by the user
	$.ajax({
		type: "get",
		dataType: "json",
		url: "/Rename",
		data: {
			fileName: currentFile, // Sends the currentFile chosen by the user to the get request so the server knows which file to open
			componentType: componentType, // Sends the componentType selected by the user to the get request so the server knows which component to rename
			componentNumber: componentNumber, // Sends the componentNumber selected by the user to the get request so the server knows which specific component to rename
			newName: $("#nameBox").val(), // Sends the new name the user wants so the server knows what to rename the component to
		},
		// Function if the get request succeeds
		success: function (data) {
			// Sends a user an alert about the status of the renaming
			alert(`Rename Status:\n\n${data.status}!`);

			// Calls on the dropDownFunction to update the view panel table
			dropDownFunction(currentFileButton);

			// Adding it to the console log and alert box
			$("#statusContainer").append(
				`<p>Renamed component: ${componentType} ${componentNumber} of file: ${currentFile} successfully</p>`
			);
			console.log(
				`Renamed component: ${componentType} ${componentNumber} of file: ${currentFile} successfully`
			);

			// Resets the value in the input field
			$("#nameBox").val("");
		},
		// function if the get request fails for any reason
		fail: function (error) {
			$("#statusContainer").append(
				`<p>Error with get request to rename a component</p>`
			);
			console.log("Error with get request to rename a component");
		},
	});
});

// Upon clicking the submit button, the program will create a new file with the information the user stored in the forms
$("#submitNewGPX").on("click", function () {
	// Sends a post request to create a new file with the information entered by the user
	$.ajax({
		// Get request to send data to /createGPX
		url: "/createGPX",
		type: "post",
		data: {
			creator: $("#addCreatorForm").val(), // Sends the creator entered by the user in the post request body
			fileName: $("#addFileNameForm").val(), // Sends the file name entered by the user in the post request body
		},
		// Function if the post request succeeds
		success: function (response) {
			// If the response was "FAIL", sends an alert, puts it in the console log and appends it to the alert box
			if (response === "FAIL") {
				alert("Writing to GPX failed! Not saving the file");
				console.log("Writing to GPX failed! Not saving the file");
				$("#statusContainer p").remove();
				$("#statusContainer").append(
					`<p>Error with creating the file, not creating the file</p>`
				);
				// Resets the creator and file name text fields
				$("#addCreatorForm").val("");
				$("#addFileNameForm").val("");
			}
			// If the response was "SUCCESS", reloads the web page so the new file is loaded into the tables
			else {
				location.reload();
			}
		},
		// Function if the post requests fails for any reason
		fail: function (error) {
			$("#statusContainer").append(
				`<p>Error with the post request to create a new file</p>`
			);
			console.log("Error with the post request to create a new file");
		},
	});
});

// Upon clicking the AddRouteButton, error checks to ensure that there is a file before bringing up the forms
$("#AddRouteButton").on("click", function () {
	// If the currentFile is empty, means a file has not been opened yet
	if (currentFile === "") {
		// Removes all items on the modal and appends an error message on the modal
		$("#RouteModal p").remove();
		$("#RouteModal label").remove();
		$("#RouteModal input").remove();
		$("#RouteModal button").remove();
		$("#AddedWaypoints label").remove();
		$("#AddRouteContainer button").remove();
		$("#RouteModal").append(
			`<p>Please choose a file before trying to add a route</p>`
		);

		// Adding error messages to the alert box and console
		$("#statusContainer p").remove();
		$("#statusContainer").append(
			`<p>Error: trying to press "Add Route" button without choosing a file to add the route to</p>`
		);
		console.log(
			`Error: trying to press "Add Route" button without choosing a file to add the route to`
		);
	} else {
		// Removing all previous items on the modal
		$("#RouteModal p").remove();
		$("#RouteModal label").remove();
		$("#RouteModal input").remove();
		$("#RouteModal button").remove();
		$("#AddedWaypoints label").remove();
		$("#AddRouteContainer button").remove();

		// Appending all necessary information on the modal
		$("#RouteModal").append(
			`<label for="NewName">Enter a new route name</label>`
		);
		$("#RouteModal").append(
			`<input type="text" class="form-control" id="newRouteName" placeholder="New Route Name"/>`
		);
		$("#RouteModal").append(
			`<button type="button" class="btn btn-primary Button AddWaypointButton" id="AddWaypointButton" data-toggle="modal" data-target="#addingWaypointsModal">Add Waypoint</button>`
		);
		$("#AddedWaypoints").append(
			`<label>Waypoints to be added to the Route:</label>`
		);
		$("#AddRouteContainer").append(
			`<button type="button" class="btn btn-primary Button" id="AddRouteSubmitButton" data-dismiss="modal"
		>Submit</button>`
		);
	}
});

// Holds the JSON strings for all waypoints added by the user for the current route button
let waypointsArray = [];
$("#waypointSubmitButton").on("click", function () {
	// Error check if the user tries to enter an empty or not a number as latitude or longitude
	if ($("#latitudeForm").val() === "") {
		alert("Cannot have an empty latitude");
	} else if ($("#longitudeForm").val() === "") {
		alert("Cannot have an empty longitude");
	} else if (isNaN(parseFloat($("#latitudeForm").val())) === true) {
		alert("Cannot have a non number latitude");
	} else if (isNaN(parseFloat($("#longitudeForm").val())) === true) {
		alert("Cannot have a non number longitude");
	} else if (
		parseFloat($("#latitudeForm").val()) <= -90 ||
		parseFloat($("#latitudeForm").val()) >= 90
	) {
		alert("Cannot have a longitude out of the range, -90 to 90");
	} else if (
		parseFloat($("#longitudeForm").val()) <= -180 ||
		parseFloat($("#longitudeForm").val()) > 180
	) {
		alert("Cannot have a latitude out of the range, -180 to 180");
	} else {
		// Storing the users information about the waypoint in JSON
		let waypointsInfo = {
			lat: $("#latitudeForm").val(),
			lon: $("#longitudeForm").val(),
		};

		// Adds the JSON waypoint to the waypointsArray as a JSON string
		waypointsArray.push(waypointsInfo);

		// Adds the waypoint added by the user to a container so the user can see what waypoints they are about to add to the route
		$("#AddedWaypoints").append(
			`<p>Latitude: ${$("#latitudeForm").val()} Longitude: ${$(
				"#longitudeForm"
			).val()}</p>`
		);
	}

	// Resetting the latitude and longitude forms
	$("#latitudeForm").val("");
	$("#longitudeForm").val("");
});

// Upon clicking the button to submit a route, goes through this sending a post request with a body containing JSON strings of waypoints and a JSON string of the route name
$("#AddRouteContainer").on("click", function () {
	// If the waypointsArray length is 0, means that the user did not upload any waypoints and sends an error message
	if (waypointsArray.length === 0) {
		console.log("Cannot submit a route without waypoints");
		$("#statusContainer p").remove();
		$("#statusContainer").append(
			`<p>Error: cannot add a route without at least one waypoint</p>`
		);
		alert("Error: Cannot add a route without at least one waypoint");
	}
	// If the waypointsArray length is not 0, means that there is a waypoint to be added and adds the route to the file
	else {
		// Gets the route name the user wants and stores it inside a JSON
		let routeName = { name: $("#newRouteName").val() };

		// Sends a post request to create a new route within the file specified by the user
		$.ajax({
			// Post request to send data to "/routeCreate" to create a new route
			url: "/routeCreate",
			type: "post",
			data: {
				fileName: currentFile, // Sends the current file the user wants to add the route to
				routeName: JSON.stringify(routeName), // Sends the route the user wants as a JSON string
			},
			// Function if the post request succeeds
			success: function (response) {
				if (response === "FAIL") {
					$("#statusContainer").append(`<p>Error adding new route to file`);
					console.log(`Error adding new route to file`);
					alert("Error adding route to the file");
				} else {
					$("#statusContainer").append(
						`<p>Successfully added new route to ${currentFile}</p>`
					);
					console.log(`Succcessfully added new route to ${currentFile}`);
				}
			},
			// Function if the post request fails for any reason
			fail: function (error) {
				$("#statusContainer").append(
					`<p>Error with the post request to add a new route</p>`
				);
				console.log("Error with the post request to add a new route");
			},
		});

		// If there are waypoints entered by the user, adds those waypoints to the route
		if (waypointsArray.length != 0) {
			// Sends a post request to add the current waypoint to the route
			$.ajax({
				type: "post",
				url: "/addWaypoint",
				data: {
					fileName: currentFile, // Sends the current file the user wants to add the waypoints to
					waypoint: waypointsArray, // Sends the array of waypoints to the post request
				},
				// Function if the post request succeeds
				success: function (response) {
					if (response === "FAIL") {
						$("#statusContainer").append(
							`<p>Error adding waypoints to route</p>`
						);
						console.log("Error adding waypoints to route");
						alert("Error adding waypoints to route");
					} else {
						$("#statusContainer").append(
							`<p>Successfully added waypoints to the new route</p>`
						);
						console.log("Successfully added waypoints to the new route");
					}
				},
				// Function if the post request fails for any reason
				fail: function (error) {
					$("#statusContainer").append(
						`<p>Error with post request to add waypoints to the route</p>`
					);
					console.log("Error with post request to add waypoints to the route");
				},
			});
		}

		// Loading the new file log table and gpx view panel
		loadFileTable();
		dropDownFunction(currentFileButton);

		// Resetting the latitude, longitude forms, the waypointsArray, the area showing the waypoints and the route name form
		$("#newRouteName").val("");
		$("#latitudeForm").val("");
		$("#longitudeForm").val("");
		waypointsArray = [];
		$("#AddedWaypoints p").remove();
	}
});

// Upon clicking the cancel button for the route, resets the latitude, longitude, and route name forms, resets the array of waypoints, and the area showing what waypoints will be added
function cancelRouteButton() {
	$("#newRouteName").val("");
	$("#latitudeForm").val("");
	$("#longitudeForm").val("");
	waypointsArray = [];
	$("#AddedWaypoints p").remove();
}

// Upon clicking the submit button for the find path, gets all the routes and tracks between the source and dest information with a specific precision
$("#findPathSubmitButton").on("click", function () {
	// Error check if the user tries to enter any incorrect inputs
	if (
		$("#sourceLatitudeForm").val() === "" ||
		$("#sourceLongitudeForm").val() === "" ||
		$("#destLatitudeForm").val() === "" ||
		$("#destLongitudeForm").val() === "" ||
		$("#deltaForm").val() === ""
	) {
		alert("Cannot enter empty values");
	} else if (
		isNaN(parseFloat($("#sourceLatitudeForm").val())) === true ||
		isNaN(parseFloat($("#sourceLongitudeForm").val())) === true
	) {
		alert("Cannot enter non number values");
	} else if (
		isNaN(parseFloat($("#destLatitudeForm").val())) === true ||
		isNaN(parseFloat($("#destLongitudeForm").val())) === true
	) {
		alert("Cannot enter non number values");
	} else if (isNaN(parseFloat($("#deltaForm").val())) === true) {
		alert("Cannot enter non number values");
	} else if (
		parseFloat($("#sourceLatitudeForm").val()) <= -90 ||
		parseFloat($("#sourceLatitudeForm").val()) >= 90
	) {
		alert("Source latitude is out of the range, -90 to 90");
	} else if (
		parseFloat($("#sourceLongitudeForm").val()) <= -180 ||
		parseFloat($("#sourceLongitudeForm").val()) > 180
	) {
		alert("Source longitude is out of the range, -180 to 180");
	} else if (
		parseFloat($("#destLatitudeForm").val()) <= -90 ||
		parseFloat($("#destLatitudeForm").val()) >= 90
	) {
		alert("Destination latitude is out of the range, -90 to 90");
	} else if (
		parseFloat($("#destLongitudeForm").val()) <= -180 ||
		parseFloat($("#destLongitudeForm").val()) > 180
	) {
		alert("Destination longitude is out of the range, -180 to 180");
	} else {
		// Making the findPathTable and header visible upon submiting the find path
		$("#FindPathTableHeader").show();
		$("#FindPathTable").show();

		// Removing any previous information that was appended onto the table
		$("#FindPathTableBody tr").remove();

		// Making an ajax call to get the array of routes and tracks that are between the inputs entered by the user
		$.ajax({
			type: "get",
			dataType: "json",
			url: "/findPath",
			data: {
				// Sends in all the form data entered by the user
				sourceLat: parseFloat($("#sourceLatitudeForm").val()),
				sourceLon: parseFloat($("#sourceLongitudeForm").val()),
				destLat: parseFloat($("#destLatitudeForm").val()),
				destLon: parseFloat($("#destLongitudeForm").val()),
				delta: parseFloat($("#deltaForm").val()),
			},
			// Function if the get request succeeds
			success: function (findPathData) {
				// If there were no route or tracks between the two points, appends to the table that no routes or tracks were found between the source and dest
				if (
					findPathData.routeList.length === 0 &&
					findPathData.trackList.length === 0
				) {
					$("#FindPathTableBody").append(`<tr>
					<th scope="row">No Routes or Tracks between the entered source and destination within the delta</th>
					</tr>`);

					$("#statusContainer p").remove();
					$("#statusContainer").append(
						`<p>Searching files for routes/tracks between: None</p>`
					);
					console.log("Searching files for routes/tracks between: None");
				}
				// If there are routes or tracks between, appends it to the table
				else {
					// Traverses through the list of routes between appending it onto the table
					$.each(findPathData.routeList, function (iterator, data) {
						data = JSON.parse(data);
						$.each(data, function (i, JSONString) {
							$("#FindPathTableBody").append(`<tr>
							<th scope="row">Route ${iterator + i + 1}</th>
							<th>${JSONString.name}</th>
							<th>${JSONString.numPoints}</th>
							<th>${JSONString.len}</th>
							<th>${JSONString.loop}</th>
							</tr>`);
						});
					});
					// Travereses through the list of tracks between appending it onto the table
					$.each(findPathData.trackList, function (iterator, data) {
						data = JSON.parse(data);
						$.each(data, function (i, JSONString) {
							$("#FindPathTableBody").append(`<tr>
							<th scope="row">Track ${iterator + i + 1}</th>
							<th>${JSONString.name}</th>
							<th>${JSONString.numPoints}</th>
							<th>${JSONString.len}</th>
							<th>${JSONString.loop}</th>
							</tr>`);
						});
					});
					$("#statusContainer p").remove();
					$("#statusContainer").append(
						`<p>Searching files for routes/tracks between: Found</p>`
					);
					console.log("Searching files for routes/tracks between: Found");
				}
			},
			// Function if the get request fails for any reason
			fail: function (error) {
				console.log("Error with get request for find path");
				$("#statusContainer p").remove();
				$("#statusContainer").append(
					`<p>Error with get request for find path</p>`
				);
			},
		});
	}

	// Resets all the forms at the end of the submit button
	$("#sourceLatitudeForm").val("");
	$("#sourceLongitudeForm").val("");
	$("#destLatitudeForm").val("");
	$("#destLongitudeForm").val("");
	$("#deltaForm").val("");
});

// Upon clicking the submit button for the submit button for the number of routes, makes a get request to get the number of routes with the length entered by the user
$("#numberOfRoutesSubmit").on("click", function () {
	// Error check for incorrect length
	if ($("#numberOfRoutesLength").val() === "") {
		alert("Cannot have an empty length");
	} else if (isNaN(parseFloat($("#numberOfRoutesLength").val())) === true) {
		alert("Cannot have a non number length");
	} else {
		// Makes an ajax get request to get the number of routes that have the length inputted by the user
		$.ajax({
			type: "get",
			dataType: "json",
			url: "/numberOfRoutesAndTracksWithLen",
			data: {
				routeLen: parseFloat($("#numberOfRoutesLength").val()), // Sends in the length to be searched entered by the user
				trackLen: 0, // Sends in 0 as the user is searching for routes
			},
			// Function if the get request succeeds
			success: function (data) {
				alert(`Number of Routes with Length: ${data.numberRoutes}`);
				console.log(`Number of Routes with Length: ${data.numberRoutes}`);
				$("#statusContainer p").remove();
				$("#statusContainer").append(
					`<p>Number of Routes with Length: ${data.numberRoutes}</p>`
				);
			},
			// Function if the get request fails for any reason and prints error messages to the appropriate places
			fail: function (error) {
				console.log("Error with get request for number of routes");
				$("#statusContainer p").remove();
				$("#statusContainer").append(
					`<p>Error with get request for number of routes</p>`
				);
			},
		});
	}

	// Restting the input field
	$("#numberOfRoutesLength").val("");
});

// Upon clicking the submit button for the submit button for the number of tracks, makes a get request to get the number of tracks with the length entered by the user
$("#numberOfTracksSubmit").on("click", function () {
	// Error check for incorrect length
	if ($("#numberOfTracksLength").val() === "") {
		alert("Cannot have an empty length");
	} else if (isNaN(parseFloat($("#numberOfTracksLength").val())) === true) {
		alert("Cannot have a non number length");
	} else {
		// Makes an ajax get request to get the number of tracks that have the length inputted by the user
		$.ajax({
			type: "get",
			dataType: "json",
			url: "/numberOfRoutesAndTracksWithLen",
			data: {
				routeLen: 0, // Sends in 0 as the user is seraching for tracks
				trackLen: parseFloat($("#numberOfTracksLength").val()), // Sends in the length to be search entered by the user
			},
			// Function if the get request succeeds
			success: function (data) {
				alert(`Number of Tracks with Length: ${data.numberTracks}`);
				console.log(`Number of Tracks with Length: ${data.numberTracks}`);
				$("#statusContainer p").remove();
				$("#statusContainer").append(
					`<p>Number of Tracks with Length: ${data.numberTracks}</p>`
				);
			},
			// Function if the get request fails for any reason and prints error messages to the appropriate places
			fail: function (error) {
				console.log("Error with get request for number of tracks");
				$("#statusContainer p").remove();
				$("#statusContainer").append(
					`<p>Error with get request for number of tracks</p>`
				);
			},
		});
	}

	// Resetting the input field
	$("#numberOfTracksLength").val("");
});
