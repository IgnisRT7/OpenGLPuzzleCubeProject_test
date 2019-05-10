#include "DebugLog.h"

OutputLogDebugger& OutputLogDebugger::Instance() {

	static OutputLogDebugger instance;
	return instance;
}

void OutputLogDebugger::AddTag(std::string tagName) {

	auto result = std::find(tagList.begin(), tagList.end(), tagName);
	if (result == tagList.end()) {
		//該当するタグが存在しないので新規追加

		//tagList.push_back(std::make_pair<std::string,int>(tagName,1 >> static_cast<int>(tagList.size())  ))
	}

}