#include "GPXParser.h"
#include "LinkedListAPI.h"
#include "GPXHelpers.h"

void parseXMLTree(GPXdoc *GPXdoc, xmlNode *root_element) {

    // For loop based off Professor Dennis' "print_element_names" function in libXmlExample.c
    for (xmlNode *node = root_element; node != NULL; node = node -> next) {
        // Ensuring text nodes are not included
        if (node -> type == XML_ELEMENT_NODE) {

            // If the current children node is "wpt"
            if (strcmp((char*)node -> name, "wpt") == 0) {
                // Gets the information for the wpt node and adds it to the waypoints list
                Waypoint *waypointStruct = getWaypointData(node);
                insertBack(GPXdoc -> waypoints, waypointStruct);
            }

            // If the current children node is "rte"
            if (strcmp((char*)node -> name, "rte") == 0) {
                // Dynamically allocates size of Route struct bytes and initializes routeStruct -> name
                Route *routeStruct = malloc(sizeof(Route));
                routeStruct -> name = malloc(strlen("") + 1);
                strcpy(routeStruct -> name, "");

                // Initializes a list for other data and waypoints of rte
                List *otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                List *waypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

                // Traversing through the siblings of the children of the current node
                for (xmlNode *siblings = node -> children; siblings != NULL; siblings = siblings -> next) {
                    // If there is a "rtept" node, gets the wpt information
                    if (strcmp((char*)siblings -> name, "rtept") == 0) {
                        // Gets the waypoint information and adds it to the list of waypoints
                        Waypoint *waypointStruct = getWaypointData(siblings);
                        insertBack(waypointList, waypointStruct);
                    }
                    // Gets the name node for the rte node
                    else if (strcmp((char*)siblings -> name, "name") == 0) {
                        routeStruct -> name = realloc(routeStruct -> name, strlen((char*)siblings -> children -> content) + 1);
                        strcpy(routeStruct -> name, (char*)siblings -> children -> content);
                    }
                    // Gets the other data for the rte node
                    else if (siblings -> type == XML_ELEMENT_NODE) {
                        GPXData *data = malloc(sizeof(GPXData) + (strlen((char*)siblings -> children -> content) + 1) * sizeof(char));
                        strcpy(data -> name, (char*)siblings -> name);
                        strcpy(data -> value, (char*)siblings -> children -> content);

                        // Adding the other data into the otherData list
                        insertBack(otherData, data);
                    }
                }
                // Adds the otherData and waypoint lists into the routeStruct structure
                routeStruct -> otherData = otherData;
                routeStruct -> waypoints = waypointList;

                // Adding to the routes list
                insertBack(GPXdoc -> routes, routeStruct);
            }

            // If the current children node is "trk"
            if (strcmp((char*)node -> name, "trk") == 0) {
                // Dynamically allocates size of Track struct bytes and initializes track -> name
                Track *trkStruct = malloc(sizeof(Track));
                trkStruct -> name = malloc(strlen("") + 1);
                strcpy(trkStruct -> name, "");

                // Initializes a list for other data and trackSegments of trk
                List *trkSegList = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
                List *otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

                // Traverses through the siblings of the children of the current node
                for (xmlNode *siblings = node -> children; siblings != NULL; siblings = siblings -> next) {
                    // Gets the name of the trk
                    if (strcmp((char*)siblings -> name, "name") == 0) {
                        trkStruct -> name = realloc(trkStruct -> name, strlen((char*)siblings -> children -> content) + 1);
                        strcpy(trkStruct -> name, (char*)siblings -> children -> content);
                    }
                    // Gets the list of track segs
                    else if (strcmp((char*)siblings -> name, "trkseg") == 0) {
                        // Creates a trkseg structure and creates a waypoint list
                        TrackSegment *trksegStruct = malloc(sizeof(TrackSegment));
                        List *waypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

                        // Gets the list of track points (waypoints)
                        for (xmlNode *childSibling = siblings -> children; childSibling != NULL; childSibling = childSibling -> next) {
                            if (strcmp((char*)childSibling -> name, "trkpt") == 0) {
                                // Gets the waypoint data and adds it to the waypointyList
                                Waypoint *waypointStruct = getWaypointData(childSibling);
                                insertBack(waypointList, waypointStruct);
                            }
                        }
                        // Setting the waypoints member in the trkseg structure to waypointsList
                        trksegStruct -> waypoints = waypointList;
                        insertBack(trkSegList, trksegStruct);
                    }
                    // Gets the other data for the trk node
                    else if (siblings -> type == XML_ELEMENT_NODE) {
                        GPXData *data = malloc(sizeof(GPXData) + (strlen((char*)siblings -> children -> content) + 1) * sizeof(char));
                        strcpy(data -> name, (char*)siblings -> name);
                        strcpy(data -> value, (char*)siblings -> children -> content);

                        // Adding the other data into the otherData list
                        insertBack(otherData, data);
                    }
                }
                // Adds the otherData list and trkseg into the trkStruct structure
                trkStruct -> segments = trkSegList;
                trkStruct -> otherData = otherData;

                // Adding to the tracks list
                insertBack(GPXdoc -> tracks, trkStruct);
            }
        }
        // Calls on the function again to run recursively sending child of the current node
        parseXMLTree(GPXdoc, node -> children);
    }
}

Waypoint *getWaypointData(xmlNode *node) {
    // Dynamically allocates size of Waypoint struct bytes and initalizes the members
    Waypoint *waypointStruct = malloc(sizeof(Waypoint));
    waypointStruct -> name = malloc(strlen("") + 1);
    strcpy(waypointStruct -> name, "");
    waypointStruct -> latitude = 0;
    waypointStruct -> longitude = 0;

    // Initializing the other data list
    List *otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    // Traversing through the attributes of the current element
    for (xmlAttr *attribute = node -> properties; attribute != NULL; attribute = attribute -> next) {
        // Gets the latitude of the waypoint node
        if (strcmp((char*)attribute -> name, "lat") == 0) {
            waypointStruct -> latitude = atof((char*)attribute -> children -> content);
        }
        // Gets the longitude of the waypoint node
        else if (strcmp((char*)attribute -> name, "lon") == 0) {
            waypointStruct -> longitude = atof((char*)attribute -> children -> content);
        }
    }

    // Traversing through the siblings of the children of the current node
    for (xmlNode *siblings = node -> children; siblings != NULL; siblings = siblings -> next) {
        // Gets the name of the waypoint node
        if (strcmp((char*)siblings -> name, "name") == 0) {
            waypointStruct -> name = realloc(waypointStruct -> name, strlen((char*)siblings -> children -> content) + 1);
            strcpy(waypointStruct -> name, (char*)siblings -> children -> content);
        }
        // Gets the other data for the way point node
        else if (siblings -> type == XML_ELEMENT_NODE) {
            GPXData *data = malloc(sizeof(GPXData) + (strlen((char*)siblings -> children -> content) + 1) * sizeof(char));
            strcpy(data -> name, (char*)siblings -> name);
            strcpy(data -> value, (char*)siblings -> children -> content);
            
            // Adds the other data into the otherData list
            insertBack(otherData, data);

            // Error checking to make sure data values are not empty strings
            if ((strcmp(data -> name, "") == 0) || (strcmp(data -> value, "") == 0)) {
                free(waypointStruct -> name);
                free(waypointStruct);
                fprintf(stderr, "Error: Other data had an empty name or value\n");
                return(NULL);
            }
        }
    }
    // Placing the other data list in the Waypoint struct
    waypointStruct -> otherData = otherData;

    // Returns the waypoint structure
    return(waypointStruct);
}

int waypointData(ListIterator waypointIterator) {
    void *waypointElement;
    int numData = 0;

    // Gets the number of children of waypoints in the waypoint list
    while((waypointElement = nextElement(&waypointIterator)) != NULL) {
        Waypoint *waypointStruct = (Waypoint*)waypointElement;
        numData += getLength(waypointStruct -> otherData);
        if (strcmp(waypointStruct -> name, "") != 0) {
            numData++;
        }        
    }

    // Returns the number of data in that list of waypoints
    return(numData);
}

// Functions and use of XML functions from http://www.xmlsoft.org/examples/tree2.c
xmlDoc *GPXdocToxmlDoc(GPXdoc *GPXDocStruct) {

    // Error checks to ensure the GPXDocStruct is not NULL
    if (GPXDocStruct == NULL) {
        fprintf(stderr, "ERROR: Invalid GPXDocStruct\n");
        return(NULL);
    }

    // Creating pointers to the XML tree and the root node of the XML tree
    xmlDocPtr doc = NULL;  
    xmlNodePtr root_node = NULL;

    // Initializing the libxml library
    LIBXML_TEST_VERSION

    // Creating an XML tree and GPX node then setting the GPX node as the root node for the created XML tree
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(doc, root_node);

    // Creating a namespace and adding it as an attribute to the GPX node
    xmlNsPtr nsPtr = xmlNewNs(root_node, BAD_CAST GPXDocStruct -> namespace, NULL);
    xmlSetNs(root_node, nsPtr);

    // Creating a variable to hold the GPX version in the GPXDocStruct
    char version[256] = "";
    sprintf(version, "%.1f", GPXDocStruct -> version);

    // Setting the version and creator attributes in the GPX node
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST version);
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST GPXDocStruct -> creator);

    // Adding the list of waypoints found in the GPXDocStruct as children of the GPX node
    addListOfWaypointsToParentNode(root_node, GPXDocStruct -> waypoints, "wpt");

    // Traversing the list of routes
    void *routeElement;
    ListIterator routeIterator = createIterator(GPXDocStruct -> routes);
    while ((routeElement = nextElement(&routeIterator)) != NULL) {

        // Creating a route node for the current route struct and making it a child of the GPX node
        xmlNodePtr routeNode = xmlNewChild(root_node, NULL, BAD_CAST "rte", BAD_CAST "");
        
        // Getting the route struct for the current route element in the list of routes
        Route *routeStruct = (Route*)routeElement;

        // If the route struct has a name thats not an empty string, adds it as a child of the routeNode
        if (strcmp(routeStruct -> name, "") != 0) {
            xmlNewChild(routeNode, NULL, BAD_CAST "name", BAD_CAST routeStruct -> name);
        }

        // Adding the list of otherData found in the routeStruct as children of the routeNode
        addListOfOtherDataToParentNode(routeNode, routeStruct -> otherData);

        // Adding the list of waypoints found in the routeStruct as children of the routeNode
        addListOfWaypointsToParentNode(routeNode, routeStruct -> waypoints, "rtept");
    }

    // Traversing the list of tracks
    void *trackElement;
    ListIterator trackIterator = createIterator(GPXDocStruct -> tracks);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {

        // Creating a track node for the current track struct and making it a child of the GPX node
        xmlNodePtr trackNode = xmlNewChild(root_node, NULL, BAD_CAST "trk", BAD_CAST "");

        // Getting the track struct for the current track element in the list of tracks
        Track *trackStruct = (Track*)trackElement;

        // If the track struct has a name thats not an empty string, adds it as a child of the trackNode
        if (strcmp(trackStruct -> name, "") != 0) {
            xmlNewChild(trackNode, NULL, BAD_CAST "name", BAD_CAST trackStruct -> name);
        }

        // Adding the list of otherData found in the trackStruct as children of the trackNode
        addListOfOtherDataToParentNode(trackNode, trackStruct -> otherData);

        // Traversing the list of segments found in the trackStruct
        void *trackSegmentElement;
        ListIterator trackSegmentIterator = createIterator(trackStruct -> segments);
        while ((trackSegmentElement = nextElement(&trackSegmentIterator)) != NULL) {
            
            // Creating a trackSegment node for the current trackSegmentStruct and making it a child of the trackNode
            xmlNodePtr trackSegmentNode = xmlNewChild(trackNode, NULL, BAD_CAST "trkseg", BAD_CAST "");
            
            // Getting the trackSegment struct for the current trackSegment element in the list of track segments
            TrackSegment *trackSegmentStruct = (TrackSegment*)trackSegmentElement;

            // Adding the list of waypoints found in the trackSegmentStruct as children of the trackSegmentNode
            addListOfWaypointsToParentNode(trackSegmentNode, trackSegmentStruct -> waypoints, "trkpt");
        }
    }

    // Returning the XML Tree
    return(doc);
}

void addListOfWaypointsToParentNode(xmlNodePtr parentNode, List *waypointList, char *nodeName) {

    // Traversing the list of waypoints in the waypointsList
    void *waypointElement;
    ListIterator waypointIterator = createIterator(waypointList);
    while ((waypointElement = nextElement(&waypointIterator)) != NULL) {

        // Creating a waypoint node for the current waypoint struct and making it a child of the parent node
        xmlNodePtr waypointNode = xmlNewChild(parentNode, NULL, BAD_CAST nodeName, BAD_CAST "");

        // Getting the waypoint struct for the current waypoint element in the list of waypoints
        Waypoint *waypointStruct = (Waypoint*)waypointElement;

        // Creating variables to hold the longitude and latitude values in the waypoint struct
        char longitude[256] = "";
        char latitude[256] = "";
        sprintf(longitude, "%f", waypointStruct -> longitude);
        sprintf(latitude, "%f", waypointStruct -> latitude);

        // Setting the longitude and latitude values to attributes in the waypoint node
        xmlNewProp(waypointNode, BAD_CAST "lat", BAD_CAST latitude);
        xmlNewProp(waypointNode, BAD_CAST "lon", BAD_CAST longitude);

        // If the waypoint struct has a name thats not an empty string, adds it as a child to the waypointNode
        if (strcmp(waypointStruct -> name, "") != 0) {
            xmlNewChild(waypointNode, NULL, BAD_CAST "name", BAD_CAST waypointStruct -> name);
        }

        // Adds the list of otherData in the waypointStruct to the waypointNode
        addListOfOtherDataToParentNode(waypointNode, waypointStruct -> otherData);
    }
}

void addListOfOtherDataToParentNode(xmlNodePtr parentNode, List *otherDataList) {

    // Traversing through the list of otherData
    void *otherDataElement;
    ListIterator otherDataIterator = createIterator(otherDataList);
    while ((otherDataElement = nextElement(&otherDataIterator)) != NULL) {

        // Getting the GPXData struct for the current otherData element
        GPXData *gpxdataStruct = (GPXData*)otherDataElement;

        // Adding the GPXData struct data as a child of the parent node
        xmlNewChild(parentNode, NULL, BAD_CAST gpxdataStruct -> name, BAD_CAST gpxdataStruct -> value);
    }
}

// Returns TRUE for valid, FALSE for invalid. 
// Functions and use of functions to validate GPX file against Schema file structure reference from http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html
bool validateXmlTreeWithSchema(xmlDoc *doc, char *gpxSchemaFile) {

    // Error checking the Schema file name
    if (gpxSchemaFile == NULL || (strcmp(gpxSchemaFile, "") == 0)) {
        fprintf(stderr, "ERROR: Empty/NULL Schema File Name\n");
        return(FALSE);
    }

    if (doc == NULL) {
        fprintf(stderr, "ERROR: Invalid xml Tree\n");
        return(FALSE);
    }

    // Variables for Schema File
    xmlSchemaPtr schemaPtr = NULL;
    xmlSchemaParserCtxtPtr contextPtr;

    // Enabling line numbers in elements contents
    xmlLineNumbersDefault(1);

    // Creating an XML Schemas parse context pointer to validate the XML file and setting callback functions for errors in the context pointer
    contextPtr = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(contextPtr, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    
    // If the contextPtr is NULL, xmlSchemaNewParserCtxt meaning an invalid Schema file
    if (contextPtr == NULL) {
        xmlSchemaFreeParserCtxt(contextPtr);
        xmlSchemaCleanupTypes();
        return(FALSE);
    }

    // Building an XML Schema structure storing inside schemaPtr to validate the XML file
    schemaPtr = xmlSchemaParse(contextPtr);
    // Freeing the context pointer
    xmlSchemaFreeParserCtxt(contextPtr);

    // Variables used to validate the XML file
    int validationReturnValue = 0;
    xmlSchemaValidCtxtPtr validContextPointer = xmlSchemaNewValidCtxt(schemaPtr);

    // Setting callback errors and warnings in the validContext pointer
    xmlSchemaSetValidErrors(validContextPointer, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    // Setting the return value to be the validation between the xmlDoc pointer and validContextPointer
    validationReturnValue = xmlSchemaValidateDoc(validContextPointer, doc);

    // If the SchemaPtr isn't NULL, frees the pointer
    if (schemaPtr != NULL) {
        xmlSchemaFree(schemaPtr);
    }

    // Checks to make sure validation worked, any value other than 0 means the validation failed
    if (validationReturnValue != 0) {
        fprintf(stderr, "xmlTree failed to validate with Schema file: %s\n", gpxSchemaFile);
        xmlSchemaCleanupTypes();
        xmlSchemaFreeValidCtxt(validContextPointer);
        return(FALSE);
    }

    // Freeing the valid context pointer, cleaning the Schema and XML parsers and dumping the memory
    xmlSchemaCleanupTypes();
    xmlSchemaFreeValidCtxt(validContextPointer);

    // Returns TRUE for a valid xmlTree
    return(TRUE);
}

bool validWaypointConstraints(List *waypointList) {

    // Traversing through the list of waypoints
    void *waypointElement;
    ListIterator waypointIterator = createIterator(waypointList);
    while ((waypointElement = nextElement(&waypointIterator)) != NULL) {

        // Getting the waypoint struct for the current waypointElement
        Waypoint *waypointStruct = (Waypoint*)waypointElement;

        // Error checking the name and list of otherData for NULL
        if (waypointStruct -> name == NULL || waypointStruct -> otherData == NULL) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }

        // Checking the list of otherData in the waypoint struct for any invalid members
        if (validOtherDataConstraints(waypointStruct -> otherData) == 0) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }
    }

    // Returns TRUE if the list of waypoints meets all the constraints of the GPXHeader.h file
    return(TRUE);
}

bool validOtherDataConstraints(List *otherDataList) {
    
    // Traversing through the list of otherData
    void *otherDataElement;
    ListIterator otherDataIterator = createIterator(otherDataList);
    while ((otherDataElement = nextElement(&otherDataIterator)) != NULL) {

        // Getting the GPXData struct for the current otherData element
        GPXData *gpxdataStruct = (GPXData*)otherDataElement;

        // Checking the members for empty strings
        if ((strcmp(gpxdataStruct -> name, "") == 0)|| (strcmp(gpxdataStruct -> value, "") == 0)) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }
    }

    // Returns TRUE if the list of otherData meets all the constraints of the GPXHeader.h file
    return(TRUE);
}

// Haversine formula comes from https://www.movable-type.co.uk/scripts/latlong.html2
float calculateHaversineFormula(Waypoint *waypoint1, Waypoint *waypoint2) {

    // Storing the longitude/latitude of the first waypoint in variables in radians
    float radiansLongitude1 = (M_PI / 180) * waypoint1 -> longitude;
    float radiansLatitude1 = (M_PI / 180) * waypoint1 -> latitude;

    // Storing the longitude/latitude of the second waypoint in variables in radians
    float radiansLongitude2 = (M_PI / 180) * waypoint2 -> longitude;
    float radiansLatitude2 = (M_PI / 180) * waypoint2 -> latitude;

    // Calculating the change in the longitudes/latitudes of the first and second waypoints and dividing by 2
    float changeInLongitude = (radiansLongitude2 - radiansLongitude1) / 2;
    float changeInLatitude = (radiansLatitude2 - radiansLatitude1) / 2;

    // Sining the change in longitude/latitude and squaring it
    changeInLongitude = sin(changeInLongitude);
    changeInLatitude = sin(changeInLatitude);
    float firstTerm = pow(changeInLatitude, 2);
    float lastTerm = pow(changeInLongitude, 2);

    // Cos both latitudes in waypoint1 and waypoint2
    float cosWaypoint1 = cos(radiansLatitude1);
    float cosWaypoint2 = cos(radiansLatitude2);

    // Calculating a
    float a = cosWaypoint1 * cosWaypoint2 * lastTerm;
    a += firstTerm;

    // Calculating c
    float c = atan2(sqrt(a), sqrt(1-a));
    c *= 2;

    // Calculating d which is the distance betweeen waypoint1 and waypoint2 in meters
    float d = 6371000 * c;

    // Returning the distance between waypoint1 and waypoint2 in meters
    return(d);
}

float lengthOfWaypoints(List *waypoints) {

    // Variable Declaration
    float waypointLength = 0;
    void *waypointElement1;
    void *waypointElement2;

    // Creating a list iterator for the list of waypoints found in the route
    ListIterator waypointIterator = createIterator(waypoints);

    // Pair of waypoints to compute the distance between
    waypointElement1 = nextElement(&waypointIterator);
    waypointElement2 = nextElement(&waypointIterator);

    // Traversing through the list of waypoints, calculating the total distance between the list of waypoints
    while (waypointElement2 != NULL) {

        // Creating two waypoint struct variables
        Waypoint *waypoint1 = (Waypoint*)waypointElement1;
        Waypoint *waypoint2 = (Waypoint*)waypointElement2;

        // Calculating the length between the two waypoints
        waypointLength += calculateHaversineFormula(waypoint1, waypoint2);

        // Moving onto the next pair of variables in the list of waypoints
        waypointElement1 = waypointElement2;
        waypointElement2 = nextElement(&waypointIterator);
    }

    // Returns the total length of the list of waypoints in meters
    return(waypointLength);
}

void dummyDelete(void *data) {
    return;
}