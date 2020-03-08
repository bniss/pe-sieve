#pragma once

#include <Windows.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <peconv.h>
#include "../utils/util.h"
#include "../utils/path_converter.h"

class ModuleDumpReport
{
public:

	ModuleDumpReport(ULONGLONG module_start, size_t module_size)
		: moduleStart(module_start), moduleSize(module_size),
		isDumped(false),
		is_corrupt_pe(false), 
		is_shellcode(false)
	{
	}

	const virtual bool toJSON(std::stringstream &outs, size_t level)
	{
		OUT_PADDED(outs, level, "\"module\" : ");
		outs << "\"" << std::hex << moduleStart << "\"" << ",\n";
		OUT_PADDED(outs, level, "\"module_size\" : ");
		outs << "\"" << std::hex << moduleSize << "\"" << ",\n";
		if (dumpFileName.length()) {
			OUT_PADDED(outs, level, "\"dump_file\" : ");
			outs << "\"" << peconv::get_file_name(dumpFileName) << "\"" << ",\n";
		}
		if (tagsFileName.length()) {
			OUT_PADDED(outs, level, "\"tags_file\" : ");
			outs << "\"" << peconv::get_file_name(tagsFileName) << "\"" << ",\n";
		}
		if (impListFileName.length()) {
			OUT_PADDED(outs, level, "\"imports_file\" : ");
			outs << "\"" << peconv::get_file_name(impListFileName) << "\"" << ",\n";
		}
		if (mode_info.length()) {
			OUT_PADDED(outs, level, "\"dump_mode\" : ");
			outs << "\"" << mode_info << "\"" << ",\n";
		}
		if (impRecMode.length()) {
			OUT_PADDED(outs, level, "\"imp_rec_result\" : ");
			outs << "\"" << impRecMode << "\"" << ",\n";
		}
		OUT_PADDED(outs, level, "\"is_shellcode\" : ");
		outs <<  std::dec << is_shellcode  << ",\n";
		if (is_corrupt_pe) {
			OUT_PADDED(outs, level, "\"is_corrupt_pe\" : ");
			outs << std::dec << is_corrupt_pe << ",\n";
		}

		OUT_PADDED(outs, level, "\"status\" : ");
		outs << std::dec << this->isDumped;
		return true;
	}

	ULONGLONG moduleStart;
	size_t moduleSize;
	bool is_corrupt_pe;
	bool is_shellcode;
	std::string impRecMode;
	bool isDumped;
	std::string mode_info;
	std::string dumpFileName;
	std::string tagsFileName;
	std::string impListFileName;
};

class ProcessDumpReport
{
public:

	ProcessDumpReport(DWORD _pid)
		: pid(_pid)
	{
	}

	~ProcessDumpReport()
	{
		deleteModuleReports();
	}

	void appendReport(ModuleDumpReport *report)
	{
		if (!report) return;
		module_reports.push_back(report);
	}

	size_t countTotal() const
	{
		return module_reports.size();
	}

	bool isFilled() const
	{
		if (countTotal()) return true;
		if (this->minidumpPath.length()) return true;
		return false;
	}

	size_t countDumped() const
	{
		size_t dumped = 0;
		std::vector<ModuleDumpReport*>::const_iterator itr = module_reports.begin();
		for (; itr != module_reports.end(); itr++) {
			ModuleDumpReport* module = *itr;
			if (module->isDumped) {
				dumped++;
			}
		}
		return dumped;
	}

	const virtual bool toJSON(std::stringstream &stream, size_t level)
	{
		stream << "{\n";
		OUT_PADDED(stream, level, "\"pid\" : ");
		stream << std::dec << getPid() << ",\n";

		OUT_PADDED(stream, level, "\"output_dir\" : \"");
		stream << escape_path_separators(outputDir) << "\",\n";
		if (minidumpPath.length()) {
			OUT_PADDED(stream, level, "\"minidump_path\" : \"");
			stream << escape_path_separators(this->minidumpPath) << "\",\n";
		}

		OUT_PADDED(stream, level, "\"dumped\" : \n");
		OUT_PADDED(stream, level, "{\n");
		//stream << " {\n";
		OUT_PADDED(stream, level + 1, "\"total\" : ");
		stream << std::dec << countTotal() << ",\n";
		OUT_PADDED(stream, level + 1, "\"dumped\" : ");
		stream << std::dec << countDumped() << ",\n";
		OUT_PADDED(stream, level, "},\n"); // scanned
		stream << list_dumped_modules(level);
		stream << "}\n";

		return true;
	}

	DWORD getPid() const { return pid; }

	std::string outputDir;
	std::string minidumpPath;

protected:

	std::string list_dumped_modules(size_t level)
	{
		std::stringstream stream;
		//summary:
		OUT_PADDED(stream, level, "\"dumps\" : [\n");
		bool is_first = true;
		std::vector<ModuleDumpReport*>::const_iterator itr;
		for (itr = module_reports.begin(); itr != module_reports.end(); ++itr) {
			ModuleDumpReport *mod = *itr;
			if (mod->isDumped) {
				if (!is_first) {
					stream << ",\n";
				}
				OUT_PADDED(stream, level + 1, "{\n");
				if (mod->toJSON(stream, level + 2)) {
					stream << "\n";
				}
				OUT_PADDED(stream, level + 1, "}");
				is_first = false;
			}
		}
		if (module_reports.size()) {
			stream << "\n";
		}
		OUT_PADDED(stream, level, "]\n");
		return stream.str();
	}

	void deleteModuleReports()
	{
		std::vector<ModuleDumpReport*>::iterator itr = module_reports.begin();
		for (; itr != module_reports.end(); itr++) {
			ModuleDumpReport* module = *itr;
			delete module;
		}
		module_reports.clear();
	}

	DWORD pid;
	std::vector<ModuleDumpReport*> module_reports;

	friend class ResultsDumper;
};