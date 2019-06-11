#pragma once

#include <parent.hpp>
#include "json.hpp"

namespace ocrmodule
{
	using nlohmann::json;

	struct SmartAlertModule
	{
		std::string guid;
		std::string name;
		std::string description;
		std::string createtime;
		std::string modifytime;
		bool enable;
		int monitorinx;
		std::string sendertype;
		std::string senderconfig;
		std::string moduletype;
		std::string moduleconfig;
	};


	void to_json(json& j, const SmartAlertModule& module);
	void from_json(const json& j, SmartAlertModule& module);

	struct ocr
	{
		std::array<int, 3> hsv;
		int sensitivity;
	};

	void to_json(json& j, const ocr& color);
	void from_json(const json& j, ocr& color);

	struct ModuleConfig
	{
		std::array<float, 4> rect;
		std::string formula;
		float threshold;
		std::vector<ocr> ocr;
	};

	void to_json(json& j, const ModuleConfig& config);
	void from_json(const json& j, ModuleConfig& config);

	struct SenderConfig
	{
		std::string url;
	};

	void to_json(json& j, const SenderConfig& sconfig);
	void from_json(const json& j, SenderConfig& sconfig);
}


