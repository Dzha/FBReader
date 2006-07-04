/*
 * Copyright (C) 2004-2006 Nikolay Pultsin <geometer@mawhrin.net>
 * Copyright (C) 2005 Mikhail Sobolev <mss@mawhrin.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <abstract/ZLFile.h>
#include <abstract/ZLDir.h>
#include <abstract/ZLInputStream.h>

#include "XMLConfig.h"
#include "XMLConfigDelta.h"
#include "XMLConfigReader.h"
#include "XMLConfigWriter.h"
#include "XMLConfigDeltaWriter.h"

static const std::string UNKNOWN_CATEGORY = "unknown";
static const std::string CHANGES_FILE = "config.changes";

void XMLConfig::load() {
	shared_ptr<ZLDir> configDir = ZLFile(myHomeDirectory + "/." + myName).directory(false);
	if (configDir.isNull()) {
		return;
	}
	std::vector<std::string> fileNames;
	configDir->collectFiles(fileNames, true);
	for (std::vector<std::string>::const_iterator it = fileNames.begin(); it != fileNames.end(); ++it) {
		ZLFile configFile(configDir->itemName(*it));
		if (configFile.extension() == "xml") {
			XMLConfigReader(*this, it->substr(0, it->length() - 4)).readDocument(configFile.inputStream());
		}
	}
	XMLConfigReader(*this, UNKNOWN_CATEGORY).readDocument(configDir->itemName(CHANGES_FILE));
}

void XMLConfig::saveAll() {
	shared_ptr<ZLDir> configDir = ZLFile(myHomeDirectory + "/." + myName).directory(true);

	if (!configDir.isNull()) {
		for (std::set<std::string>::const_iterator it = myCategories.begin(); it != myCategories.end(); ++it) {
			shared_ptr<ZLOutputStream> stream = ZLFile(configDir->itemName(*it + ".xml")).outputStream();
			if (!stream.isNull() && stream->open()) {
				XMLConfigWriter(*this, *stream, *it).write();
				stream->close();
			}
		}
	} // TODO: show error message if config was not saved
	if (myDelta != 0) {
		myDelta->clear();
	}
	// TODO: remove CHANGES_FILE
}

void XMLConfig::saveDelta() {
	if (myDelta != 0) {
		shared_ptr<ZLDir> configDir = ZLFile(myHomeDirectory + "/." + myName).directory(true);
		shared_ptr<ZLOutputStream> stream = ZLFile(configDir->itemName(CHANGES_FILE)).outputStream();
		if (!stream.isNull() && stream->open()) {
			XMLConfigDeltaWriter(*myDelta, *stream).write();
			stream->close();
		}
	}
}
