#include "stdafx.h"

#include "msgcontrol.h"
#include "json.hpp"

namespace ocrmodule
{
	using nlohmann::json;

	void to_json(json& j, const SmartAlertModule& module)
	{
		j = json{
			{ "guid", module.guid },
			{ "name", module.name },
			{ "description", module.description },
			{ "createtime", module.createtime },
			{ "modifytime", module.modifytime },
			{ "enable", module.enable },
			{ "monitorinx", module.monitorinx },
			{ "sendertype", module.sendertype },
			{ "senderconfig", module.senderconfig },
			{ "moduletype", module.moduletype },
			{ "moduleconfig", module.moduleconfig }
		};
	}

	void from_json(const json& j, SmartAlertModule& module)
	{
		module.guid = j.value("guid", "");
		module.name = j.value("name", "");
		module.description = j.value("description", "");
		module.createtime = j.value("createtime", "");
		module.modifytime = j.value("modifytime", "");
		module.enable = j.value("enable", false);
		module.monitorinx = j.value("monitorinx", 0);
		module.sendertype = j.value("sendertype", "");
		module.senderconfig = j.value("senderconfig", "");
		module.moduletype = j.value("moduletype", "");
		module.moduleconfig = j.value("moduleconfig", "");
	}

	void to_json(json& j, const ocr& ocr) {
		j = json{
			{"hsv", ocr.hsv },
			{"sensitivity", ocr.sensitivity}
		};
	};

	void from_json(const json& j, ocr& ocr) {
		ocr.hsv = j.at("hsv").get<std::array<int, 3>>();
		ocr.sensitivity = j.at("sensitivity").get<int>();
	}

	void to_json(json& j, const ModuleConfig& config) {
		j = json{
			{"rect", config.rect},
			{"formula", config.formula},
			{"threshold", config.threshold },
			{"ocr", config.ocr}
		};
	};

	void from_json(const json& j, ModuleConfig& config) {
		config.rect = j.at("rect").get<std::array<float, 4>>();
		config.formula = j.at("formula").get<std::string>();
		config.threshold = j.at("threshold").get<float>();
		config.ocr = j.at("ocr").get<std::vector<ocr>>();
		//config.color = j.value("color", std::vector<color>());
	}

	void to_json(json& j, const SenderConfig& sconfig) {
		j = json{
			{ "url", sconfig.url},
		};
	};

	void from_json(const json& j, SenderConfig& sconfig) {
		sconfig.url = j.at("url").get<std::string>();
	}
}