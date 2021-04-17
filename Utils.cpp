//
// Created by chaowyc on 3/3/2021.
//

#include "Utils.h"

void Utils::read_directory(const string& name, vector<string>& v, string& cg_file)
{
	DIR* dirp = opendir(name.c_str());
	struct dirent * dp;
	while ((dp = readdir(dirp)) != NULL) {
		string tmp_file(dp->d_name);
		if (ends_with(tmp_file, ".txt")){
			if(ends_with(tmp_file, "-cg.txt")){
				cg_file = tmp_file;
				cout << "cg file: " << tmp_file << endl;
			} else {
				v.push_back(tmp_file);
			}
		}
	}
	closedir(dirp);
}

bool Utils::ends_with(const string &str, const string &suffix)
{
	return str.size() >= suffix.size() &&
		   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int Utils::idx(int n, int i) {
	return n + i;
}

float Utils::print_mem_usage() {
	unsigned size; //       total program size
	char buf[30];
	snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
	FILE* pf = fopen(buf, "r");
	if (pf) {
		unsigned resident;//   resident set size
		unsigned share;//      shared pages
		unsigned text;//       text (code)
		unsigned lib;//        library
		unsigned data;//       data/stack
		unsigned dt;//         dirty pages (unused in Linux 2.6)
		fscanf(pf, "%u", &size);
		//	cout << std::setprecision(4) << size/1024.0 << " MB mem used" << endl;
	}
	fclose(pf);
	return size/1024.0;
}

/******************************************************************************
 * Checks to see if a directory exists. Note: This method only checks the
 * existence of the full path AND if path leaf is a dir.
 *
 * @return  >0 if dir exists AND is a dir,
 *           0 if dir does not exist OR exists but not a dir,
 *          <0 if an error occurred (errno is also set)
 *****************************************************************************/
int Utils::dir_exists(const string& path)
{
	struct stat info;

	int statRC = stat(path.c_str(), &info );
	if( statRC != 0 )
	{
		if (errno == ENOENT)  { return 0; } // something along the path does not exist
		if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
		return -1;
	}

	return ( info.st_mode & S_IFDIR ) ? 1 : 0;
}
/******************************************************************************
 * Function to check whether a file exists or not using
 * stat() function. It returns 0 if file exists at
 * given path otherwise returns -1.
 *****************************************************************************/
int Utils::file_exists(const string& path)
{
	struct stat stats;

	return stat(path.c_str(), &stats) == 0 ? 0 : -1;
}

float Utils::output_bin_vfg(VFG& vfg, const string& base_addr, const string& project) {

	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);

	string out_file_name = out_folder + "vfg.bin";
	int vfg_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (vfg_f < 0)
	{
		perror("r1");
		exit(1);
	}
	for(int i = 0; i < vfg.num_vertices(); i++){
		string var_name = "var_" + to_string(i);
		write(vfg_f, (const void *)&vfg[i].id, sizeof(int ));
		write(vfg_f, (const void *)&vfg[i].visited, sizeof(bool));
		write(vfg_f, (const void *)&vfg[i].min_parent_level, sizeof(int ));
		write(vfg_f, (const void *)&vfg[i].fat, sizeof(bool));
		write(vfg_f, (const void *)&vfg[i].topo_id, sizeof(int));
		write(vfg_f, (const void *)&vfg[i].top_level, sizeof(int));
		write(vfg_f, (const void *)&vfg[i].path_id, sizeof(int));
		write(vfg_f, (const void *)&vfg[i].dfs_order, sizeof(int));
		write(vfg_f, (const void *)&vfg[i].pre_order, sizeof(int ));
		write(vfg_f, (const void *)&vfg[i].post_order, sizeof(int ));
		write(vfg_f, (const void *)&vfg[i].kind, sizeof(NodeType));
		write(vfg_f, (const void *)&vfg[i].func_id, sizeof(int ));
		write(vfg_f, (const void *)&vfg[i].o_vid, sizeof(int ));
		write(vfg_f, (const void *)&vfg[i].removed, sizeof(bool ));
		write(vfg_f, (const void *)&vfg[i].tcs, sizeof(double ));
		write(vfg_f, (const void *)&vfg[i].mingap, sizeof(int ));
		write(vfg_f, (const void *)&var_name[0], sizeof(char)*var_name.size());

		EdgeList &succs = vfg.out_edges(i);
		for (int j = 0; j < succs.size(); ++j) {
			write(vfg_f, (const void *) &succs[j], sizeof(int));
		}
	}
	close(vfg_f);
	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

float Utils::output_bin_ig(Graph &ig, const string &base_addr, const string &project) {
	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);
	string out_file_name = out_folder + "ig.bin";
	int ig_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (ig_f < 0)
	{
		perror("r1");
		exit(1);
	}
	for(int i = 0; i < ig.num_vertices(); i++){
		if (ig[i].removed){
			continue;
		}
		write(ig_f, (const void *)&ig[i].id, sizeof(int ));
//		EdgeList &succs = ig.out_edges(i);
//		for (int j = 0; j < succs.size(); ++j) {
//			write(ig_f, (const void *) &succs[j], sizeof(int));
//		}
	}

//	for(const auto & cs_idt: ig.vfg_ref.callsite_map){
//		auto *cs_obj = cs_idt.second;
//
//		assert(cs_obj->ret_edges.size() == cs_obj->ret2output.size());
//		assert(cs_obj->call_edges.size() == cs_obj->input2arg.size());
//		for(const auto & io : cs_obj->input2output){
//			write(ig_f, (const void *) &io.first, sizeof(int ));
//			write(ig_f, (const void *) &io.second, sizeof(int ));
//
//			int vid = idx(ig.vfg_ref.num_vertices(), io.first);
//			write(ig_f, (const void *) &vid, sizeof(int));
//			vid = idx(ig.vfg_ref.num_vertices(), io.second);
//			write(ig_f, (const void *) &vid, sizeof(int));
//		}
//	}

	close(ig_f);

	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

float Utils::output_bin_grail_indices(Graph& ig, const string &base_addr, const string &project) {
	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);
	string out_file_name = out_folder + "grail_indices.bin";
//	string out_file_name2 = out_folder + "grail_indices.bin2";
//	FILE *fp = fopen(out_file_name2.c_str(), "wb");
//	if(fp == nullptr) {
//		printf("error creating file");
//		exit(-1);
//	}
	int ig_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (ig_f < 0)
	{
		perror("r1");
		exit(1);
	}
	for(int i = 0; i < ig.num_vertices(); i++){
		if (ig[i].removed){
			continue;
		}
		write(ig_f, (const void *)&i, sizeof (int));
		write(ig_f, (const void *)&ig[i].top_level, sizeof(int));
		for (int j = 0; j < ig[i].pre->size(); ++j) {
			write(ig_f, (const void *) &ig[i].pre[j], sizeof(int));
			write(ig_f, (const void *) &ig[i].middle[j], sizeof(int));
			write(ig_f, (const void *) &ig[i].post[j], sizeof(int));
		}
	}
	close(ig_f);

	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

float Utils::output_bin_pt(Graph &bbgg, PathTree &pt, Query &pt_query, const string &base_addr, const string &project) {
	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);
	string out_file_name = out_folder + "pt_bb.bin";
	int bg_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (bg_f < 0)
	{
		perror("r1");
		exit(1);
	}
	// backbone graph itself
	for(int i = 0; i < bbgg.num_vertices(); i++){
		EdgeList &succs = bbgg.out_edges(i);
		for (int j = 0; j < succs.size(); ++j) {
			write(bg_f, (const void *) &succs[j], sizeof(int));
		}
	}

	// pathtree labels
	for(int vid = 0; vid < pt.out_uncover.size(); vid++){
		vector<int>& si = pt.out_uncover[vid];
		write(bg_f, (int*) &vid, sizeof(int));
		for (int i = 0; i < si.size(); ++i) {
			write(bg_f, (const void *) &si[i], sizeof(int));
		}
	}
	for (int i = 0; i < pt.g.num_vertices(); i++) {
		write(bg_f, (int*) &i, sizeof(int));
		for(int j = 0; j < 3; j++){
			write(bg_f, (const void *) &pt.labels[i][j], sizeof(int));
		}
	}
	for (int i = 0; i < pt_query.graillabels.size(); ++i) {
		for (int j = 0; j < pt_query.grail_dim; ++j) {
			write(bg_f, (const void *) &pt_query.graillabels[i][j], sizeof(int));
		}
	}
	close(bg_f);

	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

float Utils::output_bin_summary(VFG &vfg, const string &base_addr, const string &project) {
	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);
	string out_file_name = out_folder + "summary.bin";
	int se_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (se_f < 0)
	{
		perror("r1");
		exit(1);
	}
	for(const auto & cs_idt: vfg.callsite_map){
		auto *cs_obj = cs_idt.second;

		assert(cs_obj->ret_edges.size() == cs_obj->ret2output.size());
		assert(cs_obj->call_edges.size() == cs_obj->input2arg.size());

		for(const auto & io : cs_obj->input2output){
			write(se_f, (const void *) &io.first, sizeof(int ));
			write(se_f, (const void *) &io.second, sizeof(int ));

			int vid = idx(vfg.num_vertices(), io.first);
			write(se_f, (const void *) &vid, sizeof(int));
			vid = idx(vfg.num_vertices(), io.second);
			write(se_f, (const void *) &vid, sizeof(int));
		}
	}
	close(se_f);

	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

float Utils::output_tc(VFG &vfg, unordered_map<int, vector<int>> &reached_sink_map, const string &base_addr, const string &project) {
	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);
	string out_file_name = out_folder + "tc.bin";
	int se_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (se_f < 0)
	{
		perror("r1");
		exit(1);
	}
	for(int i = 0; i < vfg.num_vertices(); i++){
		write(se_f, (const void *) &i, sizeof(int ));
		auto &reached_sink = reached_sink_map[i];
		write(se_f, (const void *) &reached_sink[0], sizeof(int) * reached_sink.size());
	}
	close(se_f);

	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

float Utils::output_fc(vector<FunctionalVFG*> &roots_func, const string &base_addr, const string &project) {
	string out_folder = base_addr + project + "/";
	if (dir_exists(out_folder) == 0){
		mkdir(out_folder.c_str(), 0766);
	}
	assert(dir_exists(out_folder) > 0);
	string out_file_name = out_folder + "fc.bin";
	int fc_f = open(out_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fc_f < 0)
	{
		perror("r1");
		exit(1);
	}
	for(const auto & root : roots_func){
//		cout << "root: " << root->func_id << endl;
		for(int i = 0; i < root->vfg.num_vertices(); i++){
			EdgeList &succs = root->vfg.out_edges(i);
			for (int j = 0; j < succs.size(); ++j) {
				write(fc_f, (const void *) &succs[j], sizeof(int));
			}
		}
	}
	close(fc_f);

	struct stat info{};
	stat(out_file_name.c_str(), &info);
	return info.st_size / 1024.0 / 1024.0;
}

//float Utils::physical_memory_used_by_process()
//{
//	FILE* file = fopen("/proc/self/status", "r");
//	int result = -1;
//	char line[128];
//
//	while (fgets(line, 128, file) != nullptr) {
//		if (strncmp(line, "VmRSS:", 6) == 0) {
//			int len = strlen(line);
//
//			const char* p = line;
//			for (; std::isdigit(*p) == false; ++p) {}
//
//			line[len - 3] = 0;
//			result = atoi(p);
//
//			break;
//		}
//	}
//
//	fclose(file);
//
//	return result / 1024.0;
//}

//int parseLine(char* line){
//	// This assumes that a digit will be found and the line ends in " Kb".
//	int i = strlen(line);
//	const char* p = line;
//	while (*p <'0' || *p > '9') p++;
//	line[i-3] = '\0';
//	i = atoi(p);
//	return i;
//}
//
//float Utils::virtual_memory_used_by_process(){ //Note: this value is in KB!
//	FILE* file = fopen("/proc/self/status", "r");
//	int result = -1;
//	char line[128];
//
//	while (fgets(line, 128, file) != NULL){
//		if (strncmp(line, "VmSize:", 7) == 0){
//			result = parseLine(line);
//			break;
//		}
//	}
//	fclose(file);
//	return result / 1024.0;
//}