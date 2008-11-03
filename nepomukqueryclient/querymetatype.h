/*
 * This file looks very weird but is the only way to get the Query metatype
 * into the service interface when generated via qdbusxml2cpp.
 */

#include "query.h"

Q_DECLARE_METATYPE(Nepomuk::Search::Query)
