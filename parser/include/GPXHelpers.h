#include "GPXParser.h"
#include "LinkedListAPI.h"

void parseXMLTree(GPXdoc *GPXdoc, xmlNode *root_element);
Waypoint *getWaypointData(xmlNode *node);
int waypointData(ListIterator waypointIterator);
xmlDoc *GPXdocToxmlDoc(GPXdoc *GPXDocStruct);
void addListOfWaypointsToParentNode(xmlNodePtr parentNode, List *waypointList, char *nodeName);
void addListOfOtherDataToParentNode(xmlNodePtr parentNode, List *otherDataList);
bool validateXmlTreeWithSchema(xmlDoc *doc, char *gpxSchemaFile);
bool validWaypointConstraints(List *waypointList);
bool validOtherDataConstraints(List *otherDataList);
float calculateHaversineFormula(Waypoint *waypoint1, Waypoint *waypoint2);
float lengthOfWaypoints(List *waypoints);
void dummyDelete(void *data);
