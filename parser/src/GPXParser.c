#include "GPXParser.h"
#include "LinkedListAPI.h"
#include "GPXHelpers.h"

GPXdoc* createGPXdoc(char* fileName) {

    // Error checking the fileName to ensure its not NULL or an empty string
    if (fileName == NULL || (strcmp(fileName, "") == 0)) {
        fprintf(stderr, "Error: empty/null file name\n");
        return(NULL);
    }

    // Initializes the libxml library
    LIBXML_TEST_VERSION

    // Declaring the GPXdoc structure and allocating size of GPXdoc structure bytes of data and initializing version
    GPXdoc *GPXDocStructure = malloc(sizeof(GPXdoc));
    GPXDocStructure -> version = 0;

    // Initializing the lists in the GPXDocStructure
    GPXDocStructure -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    GPXDocStructure -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    GPXDocStructure -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    // Variables to read the GPX file
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    // Parses the file and returns doc
    doc = xmlReadFile(fileName, NULL, 0);

    // Error checks to ensure the doc was parsable
    if (doc == NULL) {
        fprintf(stderr, "Error: could not parse file %s\n", fileName);
        freeList(GPXDocStructure -> waypoints);
        freeList(GPXDocStructure -> routes);
        freeList(GPXDocStructure -> tracks);
        free(GPXDocStructure);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return(NULL);
    }

    // Gets the root element node in the doc
    root_element = xmlDocGetRootElement(doc);

    // Traversing through the attributes of the GPX Node and storing it in the GPXDoc structure
    for (xmlAttr *attribute = root_element -> properties; attribute != NULL; attribute = attribute -> next) {
        // If the name is equal to version, gets the content of that attribute
        if (strcmp((char*)attribute -> name, "version") == 0) {
            GPXDocStructure -> version = atof((char*)attribute -> children -> content);
        }
        // If the name is equal to content, mallocs the creator pointer and gets the content
        if (strcmp((char*)attribute -> name, "creator") == 0) {
            GPXDocStructure -> creator = malloc(strlen((char*)attribute -> children -> content) + 1);
            strcpy(GPXDocStructure -> creator, (char*)attribute -> children -> content);

            // Ensuring the creator is not empty
            if ((strcmp(GPXDocStructure -> creator, "") == 0) || GPXDocStructure -> creator == NULL) {
                free(GPXDocStructure -> creator);
                freeList(GPXDocStructure -> waypoints);
                freeList(GPXDocStructure -> routes);
                freeList(GPXDocStructure -> tracks);
                free(GPXDocStructure);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                fprintf(stderr, "Error: Creator is empty or NULL\n");
                return(NULL);
            }
        }
    }
    // Gets the name space of the GPX Node and error checking for an empty string
    strcpy(GPXDocStructure -> namespace, (char *)root_element -> ns -> href);
    if (strcmp(GPXDocStructure -> namespace, "") == 0) {
        free(GPXDocStructure -> creator);
        freeList(GPXDocStructure -> waypoints);
        freeList(GPXDocStructure -> routes);
        freeList(GPXDocStructure -> tracks);
        free(GPXDocStructure);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        fprintf(stderr, "Error: Name space is empty\n");
        return(NULL);
    }

    // Traversing through the XML tree to fetch all the information in the tree, setting root_element to the next node because the GPX node information is parsed above
    root_element = root_element -> children;
    parseXMLTree(GPXDocStructure, root_element);

    // Freeing the doc and cleaning up the parser
    xmlFreeDoc(doc);
    xmlCleanupParser();

    // Returns the pointer to the GPXdoc structure
    return(GPXDocStructure);
}

char* GPXdocToString(GPXdoc* doc) {
    if (doc == NULL) {
        return(NULL);
    }

    // Creating a string pointer for waypoints, routes, tracks list
    char *waypointsList = toString(doc -> waypoints);
    char *routesList = toString(doc -> routes);
    char *tracksList = toString(doc -> tracks);

    // Creating a string variable, allocating memory for its members and the newline and null terminator and returning it
    char *string = (char*)malloc(strlen(doc -> namespace) + 40 + strlen(doc -> creator) + strlen(waypointsList) + strlen(routesList) + strlen(tracksList) + 37);
    sprintf(string, "Name Space: %s\nVersion: %f\nCreator: %s\n%s\n%s\n%s\n", doc -> namespace, doc -> version, doc -> creator, waypointsList, routesList, tracksList);
    free(waypointsList);
    free(routesList);
    free(tracksList);
    return(string);
}

void deleteGPXdoc(GPXdoc* doc) {
    if (doc == NULL) {
        return;
    }

    // Frees all the members the GPXDoc structure
    free(doc -> creator);
    freeList(doc -> waypoints);
    freeList(doc -> routes);
    freeList(doc -> tracks);
    free(doc);
}

// Iterator structure similar to Professor Dennis' in StructListDemo
int getNumWaypoints(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    }
    // Returns the number of elements in the waypoints list
    return(getLength(doc -> waypoints));
}

int getNumRoutes(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    }
    // Returns the number of elements in the routes list
    return(getLength(doc -> routes));
}

int getNumTracks(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    }
    // Returns the number of elements in the tracks list
    return(getLength(doc -> tracks));
}

int getNumSegments(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    }
    int numSegments = 0;

    // Creating a void pointer and list iterator for the track list
    void *trackElement;
    ListIterator trackIterator = createIterator(doc -> tracks);

    // Traverses through the track list with the iterator and gets the number of segments in each track element
    while ((trackElement = nextElement(&trackIterator)) != NULL) {
        // Creates a track struct with the data and gets the number of elements in the segments list and adds it to the numSegments variable
        Track *trackStruct = (Track*)trackElement;
        numSegments += getLength(trackStruct -> segments);
    }

    // Returns the numSegments representing the number of track segments in the GPX file
    return(numSegments);
}

int getNumGPXData(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    }
    int numData = 0;

    // Gets the number of children of waypoints in the waypoint list
    ListIterator waypointIterator = createIterator(doc -> waypoints);
    numData += waypointData(waypointIterator);
   
    // Gets the number of children of routes in the route list
    void *routeElement;
    ListIterator routeIterator = createIterator(doc -> routes);
    while ((routeElement = nextElement(&routeIterator)) != NULL) {
        // Getting the other data for the routes
        Route *routeStruct = (Route*)routeElement;
        numData += getLength(routeStruct -> otherData);

        // Getting the name of the route
        if (strcmp(routeStruct -> name, "") != 0) {
            numData++;
        }   

        // Gets the numData inside the waypoint list within the route structure
        ListIterator routepointIterator = createIterator(routeStruct -> waypoints);
        numData += waypointData(routepointIterator);
    }

    // Gets the number of children of tracks in track list
    void *trackElement;
    ListIterator trackIterator = createIterator(doc -> tracks);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {
        // Getting the othet data for the track
        Track *trackStruct = (Track*)trackElement;
        numData += getLength(trackStruct -> otherData);

        // Getting the name of track
        if (strcmp(trackStruct -> name, "") != 0) {
            numData++;
        }   

        // Getting the other data inside the segment data
        void *segmentElement;
        ListIterator segmentIterator = createIterator(trackStruct -> segments);
        while ((segmentElement = nextElement(&segmentIterator)) != NULL) {
            // Getting the numData inside the waypoint list within the segments structure
            TrackSegment *trksegStruct = (TrackSegment*)segmentElement;
            ListIterator trksegWaypointIterator = createIterator(trksegStruct -> waypoints);
            numData += waypointData(trksegWaypointIterator);
        }
    }
    // Returns the numData
    return(numData);
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return(NULL);
    }

    // Makes an iterator to iterate through the list of waypoints
    void *waypointElement;
    ListIterator waypointIterator = createIterator(doc -> waypoints);
    while ((waypointElement = nextElement(&waypointIterator)) != NULL) {
        // Creates a waypoint struct and compares its name member to the name parameter
        Waypoint *waypointStruct = (Waypoint*)waypointElement;
        if (strcmp(waypointStruct -> name, name) == 0) {
            // If they are equal returns the waypoint struct
            return(waypointStruct);
        }
    }
    // If no names matched the list of waypoints, returns NULL
    return(NULL);
}

Track* getTrack(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return(NULL);
    }

    // Makes an iterator to iterate through the list of tracks
    void *trackElement;
    ListIterator trackIterator = createIterator(doc -> tracks);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {
        // Creates a track struct and compares its name member to the name parameter
        Track *trackStruct = (Track*)trackElement;
        if (strcmp(trackStruct -> name, name) == 0) {
            // If they are equal returns the track struct
            return(trackStruct);
        }
    }
    // If no names matched the list of tracks, returns NULL
    return(NULL);
}

Route* getRoute(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return(NULL);
    }

    // Makes an iterator to iterate through the list of routes
    void *routeElement;
    ListIterator routeIterator = createIterator(doc -> routes);
    while ((routeElement = nextElement(&routeIterator)) != NULL) {
        // Creates a route struct and compares its name member to the name parameter
        Route *routeStruct = (Route*)routeElement;
        if (strcmp(routeStruct -> name, name) == 0) {
            // If they are equal returns the route struct
            return(routeStruct);
        }
    }
    // IF no names matched the list of routes, returns NULL
    return(NULL);
}

// These print/delete/compare functions are based off Professor Dennis' from StructListDemo
char *gpxDataToString(void *data) {
    if (data == NULL) {
        return(NULL);
    }

    // Creates a string pointer and creates a GPXData struct from the parameter
    char *string;
    GPXData *dataStructure = (GPXData*)data;

    // Allocating memory to the string +1 for NULL terminator, +2 for spaces, +1 for ":", then putting the contents in the GPXData struct into the string
    string = (char*)malloc(strlen(dataStructure -> name) + strlen(dataStructure -> value) + 1 + 2 + 1 + 12);
    sprintf(string, "Other Data:\n%s : %s", dataStructure -> name, dataStructure -> value);

    return(string);
}

void deleteGpxData(void *data) {
    if (data == NULL) {
        return;
    }

    // Creates a GPXData struct from the parameter and frees it
    GPXData *dataStructure = (GPXData*)data;
    free(dataStructure);
}

int compareGpxData(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return(1);
    }
    return(0);
}

char *waypointToString(void *data) {
    if (data == NULL) {
        return(NULL);
    }

    // Creates a string pointer and a Waypoint struct from the parameter
    char *string;
    Waypoint *waypointStruct = (Waypoint*)data;

    // Creating a string pointer for the otherData list
    char *otherDataString = toString(waypointStruct -> otherData);

    // Allocates memory for the name and other data list in the waypoint struct
    // +1 for NULL terminator, in Professor Dennis' example he represented an integer with 20 bytes and a double is 2x an ineger so +40 for each double
    // +3 for spaces, +3 for new lines, +21 for labels(name/latitude/longitude), +3 for ":", +1 for NULL terminator
    string = (char*)malloc(strlen(waypointStruct -> name) + 31 + 40 + 40 + strlen(otherDataString) + 9);

    // Puts the contents of the waypoint structure into string and returns it
    sprintf(string, "Waypoint:\nName: %s\nLatitude: %f\nLongitude: %f%s", waypointStruct -> name, waypointStruct -> latitude, waypointStruct -> longitude, otherDataString);
    free(otherDataString);
    return(string);
}

void deleteWaypoint(void *data) {
    if (data == NULL) {
        return;
    }

    // Creating a waypoint structure with the parameters
    Waypoint *waypointStruct = (Waypoint*)data;

    // Freeing any members allocated in the waypoint structure and then freeing the structure itself
    free(waypointStruct -> name);
    freeList(waypointStruct -> otherData);
    free(waypointStruct);
}

int compareWaypoints(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return(1);
    }
    return(0);
}

char *routeToString(void *data) {
    if (data == NULL) {
        return(NULL);
    }

    // Creaing a char string and making a route struct with the parameter
    char *string;
    Route *routeStruct = (Route*)data;

    // Creating string pointers for the list in the route struct
    char *waypointsList = toString(routeStruct -> waypoints);
    char *otherDataList = toString(routeStruct -> otherData);

    // Allocating memory for the members in the route struct and the formatting for the string
    string = (char*)malloc(strlen(routeStruct -> name) + strlen(waypointsList) + strlen(otherDataList) + 14);

    // Stores all the route struct data with formatting in the string pointer and returns it
    sprintf(string, "Route:\nName: %s%s%s", routeStruct -> name, waypointsList, otherDataList);
    free(waypointsList);
    free(otherDataList);
    return(string);
}

void deleteRoute(void *data) {
    if (data == NULL) {
        return;
    }

    // Creating a route struct with the parameter
    Route *routeStruct = (Route*)data;

    // Freeing the members of the route struct and the structure itself
    free(routeStruct -> name);
    freeList(routeStruct -> waypoints);
    freeList(routeStruct -> otherData);
    free(routeStruct);
}

int compareRoutes(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return(1);
    }
    return(0);
}

char *trackSegmentToString(void *data) {
    if (data == NULL) {
        return(NULL);
    }

    // Creating a char pointer and a tracksegment structure with the parameter
    char *string;
    TrackSegment *trkSeg = (TrackSegment*)data;

    // Creating a string pointer for the waypoints list
    char *waypointsList = toString(trkSeg -> waypoints);

    // Allocates memory for the members of trkseg and adds 16 for formatting and NULL terminator
    string = (char*)malloc(strlen(waypointsList) + 15);

    // Stores the tracksegment members formatted in the string variable and returns
    sprintf(string, "Track Segment:%s", waypointsList);
    free(waypointsList);
    return(string);
}

void deleteTrackSegment(void *data) {
    if (data == NULL) {
        return;
    }

    // Making a tracksegment structure with the parameter
    TrackSegment *trkSeg = (TrackSegment*)data;

    // Freeing the members of tracksegment and the structure itself
    freeList(trkSeg -> waypoints);
    free(trkSeg);
}

int compareTrackSegments(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return(1);
    }
    return(0);
}

char *trackToString(void *data) {
    if (data == NULL) {
        return(NULL);
    }

    // Creating string pointer and a track structure with parameter
    char *string;
    Track *trackStruct = (Track*)data;

    // Creates string pointers for segments and otherData list
    char *segmentsList = toString(trackStruct -> segments);
    char *otherDataList = toString(trackStruct -> otherData);

    // Allocating memory for the string with enough bytes for the members of the track struct and formatting
    string = (char*)malloc(strlen(trackStruct -> name) + strlen(segmentsList) + strlen(otherDataList) + 14);
    
    // Stores the track members in the string with formatting and returns to the user
    sprintf(string, "Track:\nName: %s%s%s", trackStruct -> name, segmentsList, otherDataList);
    free(segmentsList);
    free(otherDataList);
    return(string);
}

void deleteTrack(void *data) {
    if (data == NULL) {
        return;
    }

    // Creating a track structure with the parameter
    Track *trackStruct = (Track*)data;

    // Freeing the members of the track structure and the structure itself
    free(trackStruct -> name);
    freeList(trackStruct -> segments);
    freeList(trackStruct -> otherData);
    free(trackStruct);
}

int compareTracks(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return(1);
    }
    return(0);
}

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {

    // Error checking the file names of the GPX file and Schema file
    if (fileName == NULL || (strcmp(fileName, "") == 0)) {
        fprintf(stderr, "ERROR: Empty/NULL GPX File Name\n");
        return(NULL);
    }
    if (gpxSchemaFile == NULL || (strcmp(gpxSchemaFile, "") == 0)) {
        fprintf(stderr, "ERROR: Empty/NULL Schema File Name\n");
        return(NULL);
    }

    // Initializing the libxml library
    LIBXML_TEST_VERSION

    // Declaring the GPXdoc structure and allocating size of GPXdoc structure bytes of data and initializing version
    GPXdoc *GPXDocStruct = malloc(sizeof(GPXdoc));
    GPXDocStruct -> version = 0;

    // Initializing the lists in the GPXDocStruct
    GPXDocStruct -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    GPXDocStruct -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    GPXDocStruct -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    // Variables to read the GPX file
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    // Parses the XML file and returns an xmlDoc pointer to an XML tree
    doc = xmlReadFile(fileName, NULL, 0);

    // Error checks to ensure the XML file was parsable
    if (doc == NULL) {
        fprintf(stderr, "ERROR: XML file: %s was not parsable\n", fileName);
        freeList(GPXDocStruct -> waypoints);
        freeList(GPXDocStruct -> routes);
        freeList(GPXDocStruct -> tracks);
        free(GPXDocStruct);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return(NULL);
    }

    // Checks the validation of the GPX file, if validateXmlTreeWithSchema returns 0, means the GPX file was invalid and returns NULL
    if (validateXmlTreeWithSchema(doc, gpxSchemaFile) == 0) {
        fprintf(stderr, "GPX file: %s failed to validate with Schema file: %s\n", fileName, gpxSchemaFile);
        freeList(GPXDocStruct -> waypoints);
        freeList(GPXDocStruct -> routes);
        freeList(GPXDocStruct -> tracks);
        free(GPXDocStruct);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        xmlMemoryDump();
        return(NULL);
    }

    // Gets the root element node in the doc
    root_element = xmlDocGetRootElement(doc);

    // Traversing through the attributes of the GPX Node and storing it in the GPXDoc structure
    for (xmlAttr *attribute = root_element -> properties; attribute != NULL; attribute = attribute -> next) {
        // If the name is equal to version, gets the content of that attribute
        if (strcmp((char*)attribute -> name, "version") == 0) {
            GPXDocStruct -> version = atof((char*)attribute -> children -> content);
        }
        // If the name is equal to content, mallocs the creator pointer and gets the content
        if (strcmp((char*)attribute -> name, "creator") == 0) {
            GPXDocStruct -> creator = malloc(strlen((char*)attribute -> children -> content) + 1);
            strcpy(GPXDocStruct -> creator, (char*)attribute -> children -> content);

            // Ensuring the creator is not empty
            if ((strcmp(GPXDocStruct -> creator, "") == 0) || GPXDocStruct -> creator == NULL) {
                deleteGPXdoc(GPXDocStruct);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                xmlMemoryDump();
                fprintf(stderr, "Error: Creator is empty or NULL\n");
                return(NULL);
            }
        }
    }
    // Gets the name space of the GPX Node and error checking for an empty string
    strcpy(GPXDocStruct -> namespace, (char *)root_element -> ns -> href);
    if (strcmp(GPXDocStruct -> namespace, "") == 0) {
        deleteGPXdoc(GPXDocStruct);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        xmlMemoryDump();
        fprintf(stderr, "Error: Name space is empty\n");
        return(NULL);
    }

    // Traversing through the XML tree to fetch all the information in the tree, setting root_element to the next node because the GPX node information is parsed above
    root_element = root_element -> children;
    parseXMLTree(GPXDocStruct, root_element);

    // Freeing the xmlDoc pointer, cleaning up the Schemas type library as well as the XML parser and dumps any memory associated with the Schema
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();

    // Returns a valid GPXdoc structure
    return(GPXDocStruct);
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {

    // Error check to ensure GPXdoc is not NULL
    if (doc == NULL) {
        fprintf(stderr, "ERROR: GPXDoc is NULL\n");
        return(FALSE);
    }

    if (gpxSchemaFile == NULL || (strcmp(gpxSchemaFile, "") == 0)) {
        fprintf(stderr, "ERROR: Invalid Schema File\n");
        return(FALSE);
    }

    // Converting the GPXdoc struct into an XML tree
    xmlDoc *xmlTree = GPXdocToxmlDoc(doc);

    // Calls the validateXmlTreeWithSchema to check the validity of the xmlTree with the Schema file, a return value of 0 indicates an invalid xmlTree against the Schema file
    if (validateXmlTreeWithSchema(xmlTree, gpxSchemaFile) == 0) {
        xmlFreeDoc(xmlTree);
        xmlCleanupParser();
        return(FALSE);
    }

    // Freeing the xmlTree created to check its validity and cleaning up the XML parser as the below checks do not rely on the XML library
    xmlFreeDoc(xmlTree);
    xmlCleanupParser();

    // Error checking the namespace for NULL or empty
    if (doc -> namespace == NULL || (strcmp(doc -> namespace, "") == 0)) {
        fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
        return(FALSE);
    }

    // Error checking the creator for NULL or empty
    if (doc -> creator == NULL || (strcmp(doc -> creator, "") == 0)) {
        fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
        return(FALSE);
    }

    // Error checking all the list for NULL
    if (doc -> waypoints == NULL || doc -> routes == NULL || doc -> tracks == NULL) {
        fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
        return(FALSE);
    }

    // Checking the list of waypoints in the GPXdoc struct for any invalid members
    if (validWaypointConstraints(doc -> waypoints) == 0) {
        fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
        return(FALSE); 
    }

    // Checking the list of routes in the GPXdoc struct for any invalid members
    void *routeElement;
    ListIterator routeIterator = createIterator(doc -> routes);
    while ((routeElement = nextElement(&routeIterator)) != NULL) {

        // Getting the route struct for the current routeElement
        Route *routeStruct = (Route*)routeElement;

        // Error check name in routeStruct for NULL
        if (routeStruct -> name == NULL) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }

        // Error check the list of waypoints and list of otherData for NULL
        if (routeStruct -> waypoints == NULL || routeStruct -> otherData == NULL) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }

        // Error check to ensure list of waypoints have valid members
        if (validWaypointConstraints(routeStruct -> waypoints) == 0) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }

        // Error check to ensure list of otherData have valid members
        if (validOtherDataConstraints(routeStruct -> otherData) == 0) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }  
    }
    
    // Checking the list of tracks in the GPXdoc struct for any invalid members
    void *trackElement;
    ListIterator trackIterator = createIterator(doc -> tracks);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {

        // Getting the track struct for the current trackElement
        Track *trackStruct = (Track*)trackElement;

        // Error check name in the trackStruct for NULL
        if (trackStruct -> name == NULL) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }

        // Error check the list of segments and list of otherData for NULL
        if (trackStruct -> segments == NULL || trackStruct -> otherData == NULL) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }

        // Error check to ensure the list of segments have valid members
        void *trackSegmentElement;
        ListIterator trackSegmentIterator = createIterator(trackStruct -> segments);
        while ((trackSegmentElement = nextElement(&trackSegmentIterator)) != NULL) {

            // Getting the track segment struct for the current trackSegmentElement
            TrackSegment *trackSegmentStruct = (TrackSegment*)trackSegmentElement;

            // Error check the list of waypoints in the trackSegmentStruct for NULL
            if (trackSegmentStruct -> waypoints == NULL) {
                fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
                return(FALSE);
            }

            // Error check to ensure the list of waypoints have valid members
            if (validWaypointConstraints(trackSegmentStruct -> waypoints) == 0) {
                fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
                return(FALSE);
            }
        }

        // Error check to ensure the list of otherData have valid members
        if (validOtherDataConstraints(trackStruct -> otherData) == 0) {
            fprintf(stderr, "GPXdoc does not meet the requirements of the header file\n");
            return(FALSE);
        }
    }

    // Return TRUE meaning that the GPXdoc struct is valid by the Schema file and the requirements in the header file
    return(TRUE);
}

bool writeGPXdoc(GPXdoc* doc, char* fileName) {

    // Error check to ensure GPXdoc is not NULL
    if (doc == NULL) {
        fprintf(stderr, "ERROR: GPXDoc is NULL\n");
        return(FALSE);
    }

    // Error check to ensure the fileName is not NULL
    if (fileName == NULL || (strcmp(fileName, "") == 0)) {
        fprintf(stderr, "ERROR: File name is NULL\n");
        return(FALSE);
    }

    // Error check to ensure a period was given denoting the file extension
    if (strrchr(fileName, '.') == NULL) {
        fprintf(stderr, "ERROR: Incorrect file extension\n");
        return(FALSE);
    }

    // Error check to ensure the fileName has the correct extension
    if (strcmp(strrchr(fileName, '.'), ".gpx") != 0) {
        fprintf(stderr, "ERROR: Incorrect file extension\n");
        return(FALSE);
    }

    // Converting the GPXdoc struct into an xmlTree
    xmlDoc *xmlTree = GPXdocToxmlDoc(doc);
    
    // Writing the XML tree to the inputted fileName
    xmlSaveFormatFileEnc(fileName, xmlTree, "UTF-8", 1);
    
    // Freeing the xmlTree created and cleaning up the xml parser and dumping the memory
    xmlFreeDoc(xmlTree);
    xmlCleanupParser();
    xmlMemoryDump();

    // Returns TRUE if no errors were encountered and the file was written to correctly
    return(TRUE);
}

float getRouteLen(const Route *rt) {

    // Error check for a NULL Route
    if (rt == NULL) {
        fprintf(stderr, "ERROR: Route is NULL\n");
        return(0);
    }
    
    // Getting the total length of the list of waypoints found in the rt struct
    float routeLength = lengthOfWaypoints(rt -> waypoints);
    
    // Returns the total length of the route
    return(routeLength);
}

float getTrackLen(const Track *tr) {

    // Error check for a NULL track
    if (tr == NULL) {
        fprintf(stderr, "ERROR: Track is NULL\n");
        return(0);
    }

    // Variable Declaration
    float trackLength = 0;

    // Traversing the list of segments found in the tr struct
    void *trackSegmentElement;
    ListIterator trackSegmentIterator = createIterator(tr -> segments);
    while((trackSegmentElement = nextElement(&trackSegmentIterator)) != NULL) {

        // Getting the TrackSegment struct for the current trackSegmentElement
        TrackSegment *trackSegmentStruct = (TrackSegment*)trackSegmentElement;

        // Calculating the length of the list of waypoints found in the trackSegmentStruct
        trackLength += lengthOfWaypoints(trackSegmentStruct -> waypoints);
    }

    // Returns the total length of the track
    return(trackLength);
}

float round10(float len) {

    // Error check for a negative length
    if (len < 0) {
        fprintf(stderr, "ERROR: Length is negative\n");
        return(0);
    }

    // Getting the len without decimals and the len down to its one's column
    int roundedLen = len;
    int onesColumn = roundedLen % 10;

    // If the one's column is less than 5, rounds the number down
    if (onesColumn < 5) {
        // round down
        int difference = onesColumn;
        roundedLen -= difference;

    }
    // If the one's column is greater than or equal to 5, rounds the number up
    else {
        int difference = 10 - onesColumn;
        roundedLen += difference;
    }

    // Returns the rounded len to the nearest 10
    return(roundedLen);
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {

    // Error check for a NULL doc or negative len or delta
    if (doc == NULL || len < 0 || delta < 0) {
        fprintf(stderr, "ERROR: GPXdoc is NULL or len or delta is negative\n");
        return(0);
    }

    // Variable to keep track of the number of routes that match the inputted length
    int numRoutes = 0;

    // Traversing through the list of routes in the GPXdoc structure
    void *routeElement;
    ListIterator routeIterator = createIterator(doc -> routes);
    while ((routeElement = nextElement(&routeIterator))!= NULL) {

        // Getting the routeStruct for the current routeElement
        Route *routeStruct = (Route*)routeElement;

        // Gets the length of the current routeStruct and computing the difference in the length of the current routeStruct and inputted length
        float routeLength = getRouteLen(routeStruct);
        float differenceInLength = abs(routeLength - len);

        // If the difference in the length of the route and inputted length is within the tolerance denoted by delta, they are considered the same
        if (differenceInLength <= delta) {
            numRoutes++;
        }
    }

    // Returns the number of routes with the same length within the tolerance as specified by the parameters
    return(numRoutes);
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta) {

    // Error check for a NULL doc or negative len or delta
    if (doc == NULL || len < 0 || delta < 0) {
        fprintf(stderr, "ERROR: GPXdoc is NULL or len or delta is negative\n");
        return(0);
    }

    // Variable to keep track of the number of tracks that match the inputted length
    int numTracks = 0;

    // Traversing through the list of tracks in the GPXdoc structure
    void *trackElement;
    ListIterator trackIterator = createIterator(doc -> tracks);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {

        // Getting the track struct for the current trackElement
        Track *trackStruct = (Track*)trackElement;

        // Gets the length of the current trackStruct and computing the difference in the length of the current trackStruct and inputted length
        float trackLength = getTrackLen(trackStruct);
        float differenceInLength = abs(trackLength - len);

        // If the difference in the length of the track and inputted length is within the tolerance denoted by delta, they are considered the same
        if (differenceInLength <= delta) {
            numTracks++;
        }
    }

    // Returns the number of tracks with the same length within the tolerance as specified by the parameters
    return(numTracks);
}

bool isLoopRoute(const Route* route, float delta) {

    // Error check for NULL route or negative delta
    if (route == NULL || delta < 0) {
        fprintf(stderr, "ERROR: Route is NULL or delta is negative\n");
        return(FALSE);
    }

    // If there is less than 4 waypoints, a loop cannot be formed
    if (getLength(route -> waypoints) < 4) {
        return(FALSE);
    }

    // Getting the waypoint structures for the first and last waypoints in the list of waypoints
    Waypoint *firstWaypoint = (Waypoint*)getFromFront(route -> waypoints);
    Waypoint *lastWaypoint = (Waypoint*)getFromBack(route -> waypoints);

    // Calculating the distance between the first and last waypoint in meters
    float distanceBetween = calculateHaversineFormula(firstWaypoint, lastWaypoint);

    // If the distance between the first and last waypoint is inside of the delta tolerance, they are the same and a closed loop is formed with this route
    if (distanceBetween <= delta) {
        return(TRUE);
    }

    // Returns false if the difference between the first and last waypoint is outside of the delta tolerance
    return(FALSE);
}

bool isLoopTrack(const Track *tr, float delta) {

    // Error check for NULL track or negative delta
    if (tr == NULL || delta < 0) {
        fprintf(stderr, "ERROR: Track is NULL or delta is negative\n");
        return(FALSE);
    }

    // Variable to hold the number of waypoints in the track
    int numWaypoints = 0;

    // Traversing through the list of segments in the tr struct
    void *trackSegmentElement;
    ListIterator trackSegmentIterator = createIterator(tr -> segments);
    while ((trackSegmentElement = nextElement(&trackSegmentIterator)) != NULL) {

        // Counting the number of waypoints in the list of waypoints in the trackSegment
        TrackSegment *trackSegmentStruct = (TrackSegment*)trackSegmentElement;
        numWaypoints += getLength(trackSegmentStruct -> waypoints);
    }

    // If the total number of waypoints in the track is less than 4, a loop cannot be formed
    if (numWaypoints < 4) {
        return(FALSE);
    }

    // Getting the first and last track segments in the list of track segments
    TrackSegment *firstSegment = (TrackSegment*)getFromFront(tr -> segments);
    TrackSegment *lastSegment = (TrackSegment*)getFromBack(tr -> segments);

    // Getting the first waypoint of the first segment and the last waypoint of the last segment
    Waypoint *firstWaypoint = (Waypoint*)getFromFront(firstSegment -> waypoints);
    Waypoint *lastWaypoint = (Waypoint*)getFromBack(lastSegment -> waypoints);

    // Calculating the distance between the first and last waypoint in meters
    float distanceBetween = calculateHaversineFormula(firstWaypoint, lastWaypoint);

    // If the distance between the first and last waypoint is inside of the delta tolerance, they are the same and a closed loop is formed with this route
    if (distanceBetween <= delta) {
        return(TRUE);
    }

    // Return false if the difference between the first waypoint in the first track segment and the last waypoint in the last track segment is outside of the delta tolerance
    return(FALSE);
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

    // Error check the GPXdoc structure for NULL
    if (doc == NULL) {
        fprintf(stderr, "ERROR: GPXdoc structure is NULL\n");
        return(NULL);
    }

    // Creating a waypoint with the source longitude/latitude values
    Waypoint *sourceWaypoint = malloc(sizeof(Waypoint));
    sourceWaypoint -> longitude = sourceLong;
    sourceWaypoint -> latitude = sourceLat;

    // Creating a waypoint with the dest longitude/latitude values
    Waypoint *destWaypoint = malloc(sizeof(Waypoint));
    destWaypoint -> longitude = destLong;
    destWaypoint -> latitude = destLat;

    // Creating a list to hold routes between the specified locations
    List *routesBetweenList = initializeList(&routeToString, &dummyDelete, &compareRoutes);

    void *routeElement;
    ListIterator routeIterator = createIterator(doc -> routes);
    while ((routeElement = nextElement(&routeIterator)) != NULL) {

        // Getting the routeStruct of the current routeElement
        Route *routeStruct = (Route*)routeElement;

        // Gets the first and last waypoint of the routeStruct
        Waypoint *firstWaypoint = (Waypoint*)getFromFront(routeStruct -> waypoints);
        Waypoint *lastWaypoint = (Waypoint*)getFromBack(routeStruct -> waypoints);

        // Calculating the difference between the first waypoint and the source waypoint and the difference between the last waypoint and dest waypoint
        int sourceDifference = calculateHaversineFormula(firstWaypoint, sourceWaypoint);
        int destDifference = calculateHaversineFormula(lastWaypoint, destWaypoint);

        // If both the sourceDifference and destDifference is less than delta, means the route has the same start and end locations
        if (sourceDifference <= delta && destDifference <= delta) {
            insertBack(routesBetweenList, routeStruct);
        }
    }

    // If there are no routes between the specified location, frees the list, source/dest waypoints and returns NULL
    if (getLength(routesBetweenList) == 0) {
        printf("No routes between specified location\n");
        freeList(routesBetweenList);
        free(sourceWaypoint);
        free(destWaypoint);
        return(NULL);
    }

    // Freeing the waypoints created for the source and dest longitude/latitudes
    free(sourceWaypoint);
    free(destWaypoint);

    // Returns the list of routes between the specified locations
    return(routesBetweenList);
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

    // Error check the GPXdoc structure for NULL
    if (doc == NULL) {
        fprintf(stderr, "ERROR: GPXdoc structure is NULL\n");
        return(NULL);
    }

    // Creating a waypoint with the source longitude/latitude values
    Waypoint *sourceWaypoint = malloc(sizeof(Waypoint));
    sourceWaypoint -> longitude = sourceLong;
    sourceWaypoint -> latitude = sourceLat;

    // Creating a waypoint with the dest longitude/latitude values
    Waypoint  *destWaypoint = malloc(sizeof(Waypoint));
    destWaypoint -> longitude = destLong;
    destWaypoint -> latitude = destLat;

    // Creating a list to hold tracks between the specified locations
    List *tracksBetweenList = initializeList(&trackToString, &dummyDelete, &compareTracks);

    void *trackElement;
    ListIterator trackIterator = createIterator(doc -> tracks);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {

        // Getting the trackStruct for the current trackElement
        Track *trackStruct = (Track*)trackElement;

        // Gets the first and last trackSegments of the trackStruct
        TrackSegment *firstSegment = (TrackSegment*)getFromFront(trackStruct -> segments);
        TrackSegment *lastSegment = (TrackSegment*)getFromBack(trackStruct -> segments);

        // Gets the first waypoint of the firstSegment and the last waypoint of the lastSegment
        Waypoint *firstWaypoint = (Waypoint*)getFromFront(firstSegment -> waypoints);
        Waypoint *lastWaypoint = (Waypoint*)getFromBack(lastSegment -> waypoints);

        // Calculating the difference between the first waypoint and the source waypoint and the difference between the last waypoint and dest waypoint
        int sourceDifference = calculateHaversineFormula(firstWaypoint, sourceWaypoint);
        int destDifference = calculateHaversineFormula(lastWaypoint, destWaypoint);

        // If both the sourceDifference and destDifference is less than delta, means the track has the same start and end locations
        if (sourceDifference <= delta && destDifference <= delta) {
            insertBack(tracksBetweenList, trackStruct);
        }
    }

    // If there are no tracks between the specified locations, frees the list, source/dest waypoints and returns NULL
    if (getLength(tracksBetweenList) == 0) {
        printf("No tracks between specified location\n");
        freeList(tracksBetweenList);
        free(sourceWaypoint);
        free(destWaypoint);
        return(NULL);
    }

    // Freeing the waypoints created for the source and dest longitude/latitudes
    free(sourceWaypoint);
    free(destWaypoint);

    // Returns the list of tracks between the specified locations
    return(tracksBetweenList);
}

char* routeToJSON(const Route *rt) {
    
    // Error check rt structure for NULL
    if (rt == NULL) {
        fprintf(stderr, "ERROR: Route is NULL\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "{}");
        return(JSONString);
    }

    // Getting the name of the route and checking if its an empty name
    char *name = "";
    if (strcmp(rt -> name, "") == 0) {
        name = "None";
    }
    else {
        name = rt -> name;
    }

    // Getting whether or not the route has a loop
    char *loop = "";
    if (isLoopRoute(rt, 10) == 1) {
        loop = "true";
    }
    else {
        loop = "false";
    }

    // Allocating memory for the JSONString and entering values of the route into the string with the proper JSON format
    char *JSONString = malloc(41 + strlen(name) + 1 + 16 + 16 + strlen(loop) + 1);
    sprintf(JSONString, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, getLength(rt -> waypoints), round10(getRouteLen(rt)), loop);

    // Returns an allocated string of the route in JSON format
    return(JSONString);
}

char* trackToJSON(const Track *tr) {

    // Error check tr for NULL
    if (tr == NULL) {
        fprintf(stderr, "ERROR: Track is NULL\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "{}");
        return(JSONString);
    }

    // Getting the name of the track and checking if its an empty name
    char *name = "";
    if (strcmp(tr -> name, "") == 0) {
        name = "None";
    }
    else {
        name = tr -> name;
    }

    // Getting whether or not the route has a loop
    char *loop = "";
    if (isLoopTrack(tr, 10) == 1) {
        loop = "true";
    }
    else {
        loop = "false";
    }

    // Getting the number of waypoints in the track
    int numPoints = 0;
    void *segmentElement;
    ListIterator segmentIterator = createIterator(tr -> segments);
    while ((segmentElement = nextElement(&segmentIterator)) != NULL) {
        TrackSegment *segmentStruct = (TrackSegment*)segmentElement;

        numPoints += getLength(segmentStruct -> waypoints);
    }

    // Allocating memory for the JSONString and entering values of the track into the string with the proper JSON format
    char *JSONString = malloc(27 + strlen(name) + 1 + 16 + strlen(loop) + 1 + 100);
    sprintf(JSONString, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numPoints,round10(getTrackLen(tr)), loop);

    // Returns an allocated string of the track in JSON format
    return(JSONString);
}

char* routeListToJSON(const List *list) {

    // Error check for a NULL or empty list
    if (list == NULL || getLength((List*)list) == 0) {
        fprintf(stderr, "ERROR: Empty or NULL Route list\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "[]");
        return(JSONString);
    }

    // Allocating the JSONString to initially 2 bytes, 1 for the '[', character 1 for the NULL terminating character
    char *JSONString = malloc(2);
    strcpy(JSONString, "[");

    // Traversing through the list of routes
    void *routeElement;
    ListIterator routeIterator = createIterator((List*)list);
    while ((routeElement = nextElement(&routeIterator)) != NULL) {

        // Getting the routeStruct for the current routeElement
        Route *routeStruct = (Route*)routeElement;

        // Getting the string for the above route in JSON format
        char *routeString = routeToJSON(routeStruct);

        // Reallocating enough memory for the current JSONString as well as enough memory for the new route string in JSON format
        JSONString = realloc(JSONString, strlen(JSONString) + 1 + strlen(routeString) + 1 + 2);

        // Adding the new routeString onto the list of routes in JSON string format
        strcat(JSONString, routeString);
        strcat(JSONString, ",");

        // Freeing the string for current route
        free(routeString);
    }

    // Replaces the last comma that would be present due to the above loop with the ending ']' character for a JSON object
    JSONString[strlen(JSONString)-1] = ']';

    // Returns an allocated string of the list of routes in JSON format
    return(JSONString);
}

char* trackListToJSON(const List *list) {

    // Error check for NULL or empty list
    if (list == NULL || getLength((List*)list) == 0) {
        fprintf(stderr, "ERROR: Empty or NULL Track list\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "[]");
        return(JSONString);
    }

    // Allocating the JSONString to initially 2 bytes, 1 for the '[' character, 1 for the NULL terminating character
    char *JSONString = malloc(2);
    strcpy(JSONString, "[");

    // Traversing through the list of tracks
    void *trackElement;
    ListIterator trackIterator = createIterator((List*)list);
    while ((trackElement = nextElement(&trackIterator)) != NULL) {

        // Getting the trackStruct for the current trackElement
        Track *trackStruct = (Track*)trackElement;

        // Getting the string for the above track in JSON format
        char *trackString = trackToJSON(trackStruct);

        // Reallocating enough memory for the current JSONString as well as enough memory for the new track string in JSON format
        JSONString = realloc(JSONString, strlen(JSONString) + 1 + strlen(trackString) + 1 + 2);

        // Adding the new trackString onto the list of tracks in JSON string format
        strcat(JSONString, trackString);
        strcat(JSONString, ",");
        
        // Freeing the string for the current track
        free(trackString);
    }

    // Replaces the last comma that would be present due to the above loop with the ending ']' character for a JSON object
    JSONString[strlen(JSONString)-1] = ']';

    // Returns an allocated string of the list of tracks in JSON format
    return(JSONString);
}

char* GPXtoJSON(const GPXdoc* gpx) {

    // Error check for a NULL GPXdoc structure
    if (gpx == NULL) {
        fprintf(stderr, "ERROR: GPXdoc is NULL\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "{}");
        return(JSONString);
    }

    // Allocating memory for the JSONString and entering values of the GPXdoc into the string with the proper JSON format
    char *JSONString = malloc(110 + 16 + strlen(gpx -> creator) + 1 + 32);
    sprintf(JSONString, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx -> version, gpx -> creator, getLength(gpx -> waypoints), getLength(gpx -> routes), getLength(gpx -> tracks));

    // Returns an allocated string of the GPXdoc in JSON format
    return(JSONString);
}

void addWaypoint(Route *rt, Waypoint *pt) {

    // Error check rt and pt for NULL
    if (rt == NULL || pt == NULL) {
        fprintf(stderr, "ERROR: Route struct or Waypoint struct is NULL\n");
        return;
    }

    // Adds the waypoint to the end of the list of waypoints in the rt struct
    insertBack(rt -> waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt) {

    // Error check doc and rt for NULL
    if (doc == NULL || rt == NULL) {
        fprintf(stderr, "ERROR: GPXdoc struct or Route struct is NULL\n");
        return;
    }

    // Adds the route to the end of the list of routes in the doc struct
    insertBack(doc -> routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString) {

    // Error check gpxString for NULL
    if (gpxString == NULL) {
        fprintf(stderr, "ERORR: gpxString is NULL\n");
        return(NULL);
    }

    // Variables to be used to parse the gpxString for values required for the GPXdoc structure   
    int i = 0;
    int endingIndex = 0;

    // Cutting off any characters before first occurrence of the ':' character and moving up one character to remove the ':' character from the string
    char *firstOccurrence = strchr(gpxString, ':');
    firstOccurrence = firstOccurrence + 1;

    // Traversing through the firstOccurrence string and finding the index of the first comma which will denote the end of the version value
    for (i = 0; i < strlen(firstOccurrence); i++) {
        if (firstOccurrence[i] == ',') {
            endingIndex = i;
        }
    }

    // Storing the firstOccurence string in versionString up to the comma exclusively 
    char versionString[1024] = "";
    strncpy(versionString, firstOccurrence, endingIndex);

    // Getting the version value as a double by using the atof function on versionString
    double version = atof(versionString);

    // Cutting off any characters before the second occurrence of the ':' character and moving up one character to remove the ':' and '"' character from the string
    char *secondOccurrence = strrchr(gpxString, ':');
    secondOccurrence = secondOccurrence + 2;

    // Traversing through the secondOccurrence string and finding the index of the first '"' which will denote the end of the creator value
    for (i = 0; i < strlen(secondOccurrence); i++) {
        if (secondOccurrence[i] == '"') {
            endingIndex = i;
        }
    }

    // Storing the secondOccurrence string in creator up to the '"'
    char creator[1024] = "";
    strncpy(creator, secondOccurrence, endingIndex);
    
    // Declaring a GPXdoc structure and allocating size of GPXdoc structure bytes of memory
    GPXdoc *docStruct = malloc(sizeof(GPXdoc));

    // Initializing the namespace and version
    strcpy(docStruct -> namespace, "http://www.topografix.com/GPX/1/1");
    docStruct -> version = version;

    // Allocating memory to the creator and setting its value to the creator value in the JSON string
    docStruct -> creator = malloc(strlen(creator) + 1);
    strcpy(docStruct -> creator, creator);

    // Initializing the lists in the docStruct
    docStruct -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    docStruct -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    docStruct -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    // Returns the docStruct filled with contents from the JSON string
    return(docStruct);
}

Waypoint* JSONtoWaypoint(const char* gpxString) {

    // Error check gpxString for NULL
    if (gpxString == NULL) {
        fprintf(stderr, "ERROR: gpxString is NULL\n");
        return(NULL);
    }

    // Variables to be used to parse the gpxString for values required for the GPXdoc structure   
    int i = 0;
    int endingIndex = 0;

    // Cutting off any characters before first occurrence of the ':' character and moving up one character to remove the '"' character from the string
    char *firstOccurrence = strchr(gpxString, ':');
    firstOccurrence = firstOccurrence + 2;

    // Traversing through the firstOccurrence string and finding the index of the first comma which will denote the end of the latitude value
    for (i = 0; i < strlen(firstOccurrence); i++) {
        if (firstOccurrence[i] == ',') {
            endingIndex = i - 1;
        }
    }

    // Storing the firstOccurrence string up to the comma and storing the float value in latitude with the atof funciton
    char latitudeString[1024] = "";
    strncpy(latitudeString, firstOccurrence, endingIndex);
    double latitude = atof(latitudeString);

    // Cutting off any characters before the second occurrence of ':' and moving up one character to remove the '"' character
    char *secondOccurrence = strrchr(gpxString, ':');
    secondOccurrence = secondOccurrence + 2;

    // Traversing through the secondOccurrence string and finding the index of the '}' character which will denote the end of the longitude value
    for (i = 0; i < strlen(secondOccurrence); i++) {
        if (secondOccurrence[i] == '}') {
            endingIndex = i - 1;
        }
    }

    // Storing the secondOccurrence string up to the '}' character and storing the float value in longitude with the atof function
    char longitudeString[1024] = "";
    strncpy(longitudeString, secondOccurrence, endingIndex);
    double longitude = atof(longitudeString);

    // Declaring a Waypoint structure and allocating size of Waypoint structure bytes of memory to it
    Waypoint *waypointStruct = malloc(sizeof(Waypoint));

    // Initializing a list of otherData in the waypointStruct
    waypointStruct -> otherData  = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    // Allocating memory to the name member in the waypointStruct and setting its value to an empty string
    waypointStruct -> name = malloc(1);
    strcpy(waypointStruct -> name, "");

    // Initializing the longitude and latitude values of the waypointStruct to the values found in the JSON string
    waypointStruct -> longitude = longitude;
    waypointStruct -> latitude = latitude;

    // Returns the waypointStruct filled with contents from the JSON string
    return(waypointStruct);
}

Route* JSONtoRoute(const char* gpxString) {

    // Error check gpxString for NULL
    if (gpxString == NULL) {
        fprintf(stderr, "ERROR: gpxString is NULL\n");
        return(NULL);
    }

    // Variables to be used to parse the gpxString for values required for the GPXdoc structure   
    int i = 0;
    int endingIndex = 0;

    // Cutting off any characters before first occurrence of the ':' character and moving up two characters to remove the ':' and '"' characters from the string
    char *firstOccurrence = strchr(gpxString, ':');
    firstOccurrence = firstOccurrence + 2;

    // Traversing through the firstOccurrence string and finding the index of the first comma which will denote the end of the latitude value
    for (i = 0; i < strlen(firstOccurrence); i++) {
        if (firstOccurrence[i] == '"') {
            endingIndex = i;
        }
    }

    // Storing the firstOccurrence string up to the '"' character
    char name[1024] = "";
    strncpy(name, firstOccurrence, endingIndex);

    // Declaring a Route structure and allocating size of Route structure bytes of memory
    Route *routeStruct = malloc(sizeof(Route));

    // Allocating memory to the name member and settings its value found in the JSON string
    routeStruct -> name = malloc(strlen(name) + 1);
    strcpy(routeStruct -> name, name);

    // Initializing a list of waypoints and otherData for the members found in the Route structure
    routeStruct -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    routeStruct -> otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    // Returns the routeStruct filled with contents from the JSON string
    return(routeStruct);
}

// Function to take in a GPX file name and return the GPX node as a JSON string
char *GPXFiletoJSON(char *fileName);
char *GPXFiletoJSON(char *fileName) {
    // Creates a GPXdoc structure from the GPX file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Creating a JSONString for the GPX attributes
    char *JSONString;

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the GPX attributes within the file
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        JSONString = GPXtoJSON(GPXDocStruct);
    }
    // Else file is invalid and returns empty curly braces
    else {
        JSONString = malloc(3);
        strcpy(JSONString, "{}");
    }
    deleteGPXdoc(GPXDocStruct);
    return(JSONString);
}

// Function to take in a GPX file name and returns the array of JSON string routes
char *GPXFiletoRouteListJSON(char *fileName);
char *GPXFiletoRouteListJSON(char *fileName) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file, converts the route list in GPXdoc into a JSON object and returns it
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Creating a JSONString for the list of routes in the file name
    char *JSONString;

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the JSONString for the list of routes in the file name
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        JSONString = routeListToJSON(GPXDocStruct -> routes);
    }
    // Else file is invalid and returns empty square brackets
    else {
        JSONString = malloc(3);
        strcpy(JSONString, "[]");
    }


    deleteGPXdoc(GPXDocStruct);
    return(JSONString);
}

// Function to take in a GPX file name and returns the array of JSON string tracks
char *GPXFiletoTrackListJSON(char *fileName);
char *GPXFiletoTrackListJSON(char *fileName) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Creating a JSONString for the list of tracks in the file name
    char *JSONString;

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the JSONString for the list of tracks in the file name
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        JSONString = trackListToJSON(GPXDocStruct -> tracks);
    }
    // Else file is invalid and returns empty square brackets
    else {
        JSONString = malloc(3);
        strcpy(JSONString, "[]");
    }

    deleteGPXdoc(GPXDocStruct);
    return(JSONString);
}

// Function that converts a GPXData struct into a JSON string
char *GPXDataToJSON(const GPXData *data);
char *GPXDataToJSON(const GPXData *data) {

    // Error check the GPXdata structure for NULL
    if (data == NULL) {
        fprintf(stderr, "ERROR: Route is NULL\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "{}");
        return(JSONString);
    }

    // Checking to ensure the name in the GPXdata is not NULL
    if (strcmp(data -> name, "") == 0) {
        fprintf(stderr, "ERROR: GPXdata name is an empty string\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "{}");
        return(JSONString);
    }

    // Checking to ensure the value in the GPXdata is not NULL
    if (strcmp(data -> value, "") == 0) {
        fprintf(stderr, "ERROR: GPXdata value is an empty string\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "{}");
        return(JSONString);
    }

    // Allocating memory to the string +1 NULL terminator, enough memory for the JSON characters and, enough memory for the name and value and storing the values in the string
    char *JSONString = malloc(30 + strlen(data -> name) + strlen(data -> value));
    sprintf(JSONString, "{\"%s\":\"%s\"}", data -> name, data -> value);

    // Returns an allocated string of the GPXdata in JSON fromat
    return(JSONString);
}

// Function to take in a GPX file name, returning an an array of GPXdata
char *GPXDataListToJSON(const List *list);
char *GPXDataListToJSON(const List *list) {

    // Error check for a NULL or empty list
    if (list == NULL || getLength((List*)list) == 0) {
        fprintf(stderr, "ERROR: Empty or NULL GPXData list\n");
        char *JSONString = malloc(3);
        strcpy(JSONString, "[]");
        return(JSONString);
    }

    // Allocating the JSONString to initially 2 bytes, 1 for the '[', character 1 for the NULL terminator
    char *JSONString = malloc(2);
    strcpy(JSONString, "[");

    // Traversing through the list of other data
    void *dataElement;
    ListIterator dataIterator = createIterator((List*)list);
    while ((dataElement = nextElement(&dataIterator)) != NULL) {

        // Getting the GPXData for the current dataElement
        GPXData *dataStruct = (GPXData*)dataElement;

        // Getting the string for the above GPXData in JSON format
        char *dataString = GPXDataToJSON(dataStruct);

        // Reallocating enough memory for the current JSONString as well as enough memory for the new data stirng in JSON format
        JSONString = realloc(JSONString, strlen(JSONString) + 1 + strlen(dataString) + 1 + 2);

        // Adding the new dataString onto the list of GPXData in JSON string format
        strcat(JSONString, dataString);
        strcat(JSONString, ",");

        // Freeing the string for current data
        free(dataString);
    }

    // Replaces the last comma that would be present due to the above loop with the ending ']' character for a JSON object
    JSONString[strlen(JSONString)-1] = ']';

    // Returns an allocated string of the list of GPXData in JSON format
    return(JSONString);
}

// Function to take in a GPX file name, returning an array of an array of JSONStrings holding GPXData for each route
char *GPXFiletoRouteGPXDataListJSON(char *fileName);
char *GPXFiletoRouteGPXDataListJSON(char *fileName) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Creating a JSONString for an array of list of GPXData in each route and allocating 2 bytes for NULL terminator and '[' character
    char *JSONString = malloc(2);
    strcpy(JSONString, "[");

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the JSONString for the other data in each route
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        
        // Traversing through the list of routes
        void *routeElement;
        ListIterator routeIterator = createIterator((List*)GPXDocStruct -> routes);
        while ((routeElement = nextElement(&routeIterator)) != NULL) {

            // Getting the routeStruct for the current routeElement
            Route *routeStruct = (Route*)routeElement;

            // Getting the data string for the route in JSON format
            char *dataString = GPXDataListToJSON(routeStruct -> otherData);

            // Reallocating enough memory for the current JSONString as well as enough memory for the new GPXData string in JSON format
            JSONString = realloc(JSONString, strlen(JSONString) + 1 + strlen(dataString) + 1 + 2);

            // Adding the new dataString onto the array of dataStrings in JSON string format
            strcat(JSONString, dataString);
            strcat(JSONString, ",");

            // Freeing the string for the current data
            free(dataString);
        }

        // Replaces the last comma with the ending ']' for the JSON format
        JSONString[strlen(JSONString)-1] = ']';
    }
    // Else file is invalid and returns an empty array
    else {
        JSONString = malloc(3);
        strcpy(JSONString, "[]");
    }
    deleteGPXdoc(GPXDocStruct);
    return(JSONString);
}

// Function to take in a GPX file name, returning an array of an array of JSONStrings holding GPXData for each track
char *GPXFiletoTrackGPXDataListJSON(char *fileName);
char *GPXFiletoTrackGPXDataListJSON(char *fileName) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Creating a JSONString for an array of list of GPXData in each track and allocating 2 bytes for NULL terminator and "[" character
    char *JSONString = malloc(2);
    strcpy(JSONString, "[");

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the JSONString for the other data in each track
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {

        // Traversing through the list of tracks
        void *trackElement;
        ListIterator trackIterator = createIterator((List*)GPXDocStruct -> tracks);
        while ((trackElement = nextElement(&trackIterator)) != NULL) {

            // Getting the trackStruct for the current trackElement
            Track *trackStruct = (Track*)trackElement;

            // Getting the data stirng for the track in JSON format
            char *dataString = GPXDataListToJSON(trackStruct -> otherData);

            // Reallocating enough memory for the current JSONString as well as enough memory for the new GPXData string in JSON format
            JSONString = realloc(JSONString, strlen(JSONString) + 1 + strlen(dataString) + 1 + 2);

            // Adding the new dataString onto the array of dataStrings in JSON string format
            strcat(JSONString, dataString);
            strcat(JSONString, ",");

            // Freeing the string for the current data
            free(dataString);
        }

        // Replaces the last comma with the ending ']' for the JSON format
        JSONString[strlen(JSONString)-1] = ']';
    }
    // Else file is invalid and returns an empty array
    else {
        JSONString = malloc(3);
        strcpy(JSONString, "[]");
    }
    deleteGPXdoc(GPXDocStruct);
    return(JSONString);
}

// Function to take in a GPX file name, new name, component type and number, will go into the GPX file changing the name of the component specified by the user
int renameGPXComponent(char *fileName, char *newName, char *componentType, int componentNumber);
int renameGPXComponent(char *fileName, char *newName, char *componentType, int componentNumber) {
        
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the changes the name of the component specified by the user
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        // If the component type is "Route", means that the user is trying to change the name of a route
        if (strcmp(componentType, "Route") == 0) {

            // Creates variables to iterate through the list of routes and store the specific route
            void *routeElement;
            ListIterator routeIterator = createIterator((List*)GPXDocStruct -> routes);

            // Traverses to the route specified by the user
            for (int i = 0; i < componentNumber; i++) {
                routeElement = nextElement(&routeIterator);
            }

            // Getting the route struct for the current routeElement
            Route *routeStruct = (Route*)routeElement;

            // Reallocating enough memory for the new name in the routeStruct and storing the new name in it
            routeStruct -> name = realloc(routeStruct -> name, strlen(newName) + 1);
            strcpy(routeStruct -> name, newName);
        }
        // Else the component type is "Track" meaning that the user is trying to change the name of a track
        else {

            // Creates variables to iterate through the list of tracks and store the specific track
            void *trackElement;
            ListIterator trackIterator = createIterator((List*)GPXDocStruct -> tracks);

            // Traverses to the track specified by the user
            for (int i = 0; i < componentNumber; i++) {
                trackElement = nextElement(&trackIterator);
            }

            // Getting the track struct for the current trackElement
            Track *trackStruct = (Track*)trackElement;

            // Reallocating enough memory for the new name in the trackStruct and storing the new name in it
            trackStruct -> name = realloc(trackStruct -> name, strlen(newName) + 1);
            strcpy(trackStruct -> name, newName);
        }
    
    }
    // Else the file is invalid and returns 0 for invalid
    else {
        fprintf(stderr, "File is invalid\n");
        return(0);
    }

    // Validates the updated GPXdoc, if its valid, saves the updated GPXdoc to the file
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        // Writes the updated GPX
        int returnValue = writeGPXdoc(GPXDocStruct, fileName);   

        // If the returnValue is FALSE, means there was an error writing to the file and returns 0 for invalid
        if (returnValue == FALSE) {
            fprintf(stderr, "Changing the component name failed!\n");
            return(0);
        }
    }
    // If the updated GPXdoc is invalid, returns 0 for invalid
    else {
        fprintf(stderr, "Updated GPXdoc is invalid\n");
        return(0);
    }
    // Return 1 meaning renaming was successful
    deleteGPXdoc(GPXDocStruct);
    return(1);
}

// Function to take in a file name and GPX node information and creates a new file based off the information
int createGPXFile(char *JSONGPX, char *fileName);
int createGPXFile(char *JSONGPX, char *fileName) {
    GPXdoc *GPXDocStruct = JSONtoGPX(JSONGPX);

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and writes the GPXdoc to the file entered by the user
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        // Writes the updated GPXdoc and saves the return value
        int returnValue = writeGPXdoc(GPXDocStruct, fileName);

        // If the returnValue is FALSE, means there was an error writing to the file and returns 0 for invalid
        if (returnValue == FALSE) {
            fprintf(stderr, "Writing to new file failed!\n");
            deleteGPXdoc(GPXDocStruct);
            return(0);
        }
    }
    // Else, validation is FALSE, so returns 0 with an error
    else {
        fprintf(stderr, "Invalid GPXdoc\n");
        deleteGPXdoc(GPXDocStruct);
        return(0);
    }

    // If everything was successful, returns 1 for success and frees the GPXdoc
    deleteGPXdoc(GPXDocStruct);
    return(1);
}

// Adds the route the user wanted to add to the end of the specified file
int addRouteToFile(char *fileName, char *routeJSON);
int addRouteToFile(char *fileName, char *routeJSON) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Getting the route information from the JSON string sent and adds it to the GPXdoc
    Route *routeInformation = JSONtoRoute(routeJSON);
    addRoute(GPXDocStruct, routeInformation);

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and adds the route to the file
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        // Writes the updated GPXdoc and saves the return value
        int returnValue = writeGPXdoc(GPXDocStruct, fileName);

        // If the returnValue is FALSE, means there was an error writing to the file and returns 0 for fail
        if (returnValue == FALSE) {
            fprintf(stderr, "Adding route to the file failed\n");
            deleteRoute(routeInformation);
            return(0);
        }
    }
    // Else file is invalid and returns 0 for invalid
    else {
        fprintf(stderr, "Invalid GPXdoc\n");
        deleteRoute(routeInformation);
        return(0);
    }

    // If adding the route to the file was successful, returns 1 for success
    deleteRoute(routeInformation);
    return(1);
}

// Adds a waypoint to the last route in the file name which should be the newly added route above
int addWaypointToRoute(char *fileName, char *waypointJSON);
int addWaypointToRoute(char *fileName, char *waypointJSON) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    // Getting the waypoint infromation from the JSON string and the route struct at the end of the GPXdoc
    Waypoint *waypointInformation = JSONtoWaypoint(waypointJSON);
    Route *routeStruct = getFromBack(GPXDocStruct -> routes);

    // Adds the waypoint to the end of the routeStruct
    addWaypoint(routeStruct, waypointInformation);

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and adds the waypoint to the route then saving to the file
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        // Writes the updated GPXdoc and saves the return value
        int returnValue = writeGPXdoc(GPXDocStruct, fileName);

        // If the returnValue is FALSE, means there was an error writing to the file and returns 0 for invalid
        if (returnValue == FALSE) {
            fprintf(stderr, "Adding waypoint to route then adding to file failed\n");
            deleteWaypoint(waypointInformation);
            return(0);
        }
    }
    // Else file is invalid and returns 0 for invalid
    else {
        fprintf(stderr, "Invalid GPXdoc\n");
        deleteWaypoint(waypointInformation);
        return(0);
    }

    // If adding waypoint to the route was successful, returns 1 for success
    deleteWaypoint(waypointInformation);
    return(1);
}

// Function that returns a list of JSON strings containing the routes between for that particular file
char *routeListOfRoutesBetween(char *fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta);
char *routeListOfRoutesBetween(char *fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    char *routesBetweenString = "";

    // Validates the file against GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the list of routes that are between the user inputs
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {

        // Gets the list of routes between the points and stores it as a JSON string
        List *routesBetween = getRoutesBetween(GPXDocStruct, sourceLat, sourceLong, destLat, destLong, delta);
        routesBetweenString = routeListToJSON(routesBetween);
    }
    // Else file is invalid and returns an empty object
    else {
        fprintf(stderr, "Invalid GPXdoc\n");
        deleteGPXdoc(GPXDocStruct);
        return(routesBetweenString);
    }
    // If everything is successful, deletes the GPXdoc and returns the JSON string containing the routes between
    deleteGPXdoc(GPXDocStruct);
    return(routesBetweenString);
}

// Function that returns a list of JSON strings containing the tracks between for that particular file
char *trackListOfRoutesBetween(char *fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta);
char *trackListOfRoutesBetween(char *fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");

    char *tracksBetweenString = "";

    // Validates the file against GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the list of tracks that are between the user inputs
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {

        // Gets the list of tracks between the points and stores it as a JSON string
        List *tracksBetween = getTracksBetween(GPXDocStruct, sourceLat, sourceLong, destLat, destLong, delta);
        tracksBetweenString = routeListToJSON(tracksBetween);
    }
    // Else file is invalid and returns an empty object
    else {
        fprintf(stderr, "Invalid GPXdoc\n");
        deleteGPXdoc(GPXDocStruct);
        return(tracksBetweenString);
    }
    // If everything is successful, deletes the GPXdoc and returns the JSON string containing the tracks between
    deleteGPXdoc(GPXDocStruct);
    return(tracksBetweenString);
}

int numberOfRoutesWithLengthFromFile(char *fileName, float len, float delta) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");
    int numRoutes = 0;

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the number of routes with the length inputted
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        numRoutes += numRoutesWithLength(GPXDocStruct, len, delta);
    }
    // Else file is invalid and returns 0 for invalid
    else {
        fprintf(stderr, "ERROR: Invalid GPXdoc");
        deleteGPXdoc(GPXDocStruct);
        return(0);
    }

    // Returns the number of routes and deletes the GPXdoc
    deleteGPXdoc(GPXDocStruct);
    return(numRoutes);
}

int numberOfTracksWithLengthFromFile(char *fileName, float len, float delta) {
    // Creates a GPXdoc structure and validates against the gpx.xsd file
    GPXdoc *GPXDocStruct = createValidGPXdoc(fileName, "parser/src/gpx.xsd");
    int numTracks = 0;

    // Validates the file against the GPXParser.h and gpx.xsd schema file
    // If the validation is TRUE, means that the file is valid and gets the number of tracks with the length inputted
    if (validateGPXDoc(GPXDocStruct, "parser/src/gpx.xsd") == TRUE) {
        numTracks += numTracksWithLength(GPXDocStruct, len, delta);
    }
    // Else file is invalid and returns 0 for invalid
    else {
        fprintf(stderr, "ERROR: Invalid GPXdoc");
        deleteGPXdoc(GPXDocStruct);
        return(0);
    }

    // Returns the number of tracks and deletes the GPXdoc
    deleteGPXdoc(GPXDocStruct);
    return(numTracks);

}