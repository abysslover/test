/*
 * IOUtils.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: Euncheon Lim @ Max-Planck-Institute for Developmental Biology
 *            : under the guidance of Prof. Detlef Weigel and Prof. Daniel Huson
 *      Email : abysslover@gmail.com
 *    License : GPLv3
 */

#include "IOUtils.hpp"

namespace castle {
const string IOUtils::illumina_asciis = "JKLMNOPQRSTUVWXYZ[\\]^_`abcdefgh";
const string IOUtils::sanger_asciis = "!\"#$%&'()*+,-./0123456789:";
const string IOUtils::PBWT_EXT = ".pbwt";
const string IOUtils::PRBWT_EXT = ".prbwt";
const string IOUtils::BWT_EXT = ".bwt";
const string IOUtils::RBWT_EXT = ".rbwt";
const string IOUtils::SAI_EXT = ".sai";
const string IOUtils::RSAI_EXT = ".rsai";
const string IOUtils::GZIP_EXT = ".gz";
const string IOUtils::POP_PREFIX = ".pop";
const string IOUtils::BSAI_EXT = ".bsai";
const string IOUtils::RBSAI_EXT = ".rbsai";
uint64_t IOUtils::BLOCK_SIZE = 2 * MEGA;

uint8_t IOUtils::EncodingMap[] = {
	// 0 - 15
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 16 - 31
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 32 - 47
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /*'-'*/, 0, 0,
	// 48 - 63
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 64 - 79
	0, 1 /* 'A' */, 0, 2 /* 'C' */, 0, 0, 0, 3 /* 'G' */, 0, 0, 0, 0, 0, 0, 0, 0,
	// 80 - 95
	0, 0, 0, 0, 4 /* 'T' */, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 96 - 111
	0, 1 /* 'a' */, 0, 2 /* 'c' */, 0, 0, 0, 3 /* 'g' */, 0, 0, 0, 0, 0, 0, 0, 0,
	// 112 - 127
	0, 0, 0, 0, 4 /* 't' */, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 128 - 143
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 144 - 159
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 160 - 175
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 176 - 191
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 192 - 207
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 208 - 223
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 224 - 239
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 240 - 255
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char IOUtils::DecodingMap[] = { 'N', 'A', 'C', 'G', 'T' };
//uint64_t IOUtils::MAX_OPEN_FILE = 2047;
//uint64_t IOUtils::MAX_OPEN_FILE_PRIME = 2039;
//constexpr uint64_t IOUtils::MAX_OPEN_FILE = 2047;

IOUtils::IOUtils() {
}

IOUtils::~IOUtils() {
}
string IOUtils::expand_home(const string& a_path) {
	if (a_path.empty() || '~' != a_path[0]) {
		return a_path;
	}
	string expanded_path(a_path);
	char const* home = getenv("HOME");
	if (home || ((home = getenv("USERPROFILE")))) {
		expanded_path.replace(0, 1, home);
	} else {
		char const *hdrive = getenv("HOMEDRIVE"), *hpath = getenv("HOMEPATH");
		expanded_path.replace(0, 1, string(hdrive) + hpath);
	}
	return expanded_path;
}
Type_Of_Format IOUtils::get_file_format(const string& a_path) {
	if (bfs::is_directory(a_path)) {
		return NON_SEQUENCING;
	}
	ifstream an_inputstream(a_path.c_str(), ios::binary);
	string line;
	map<char, int64_t> delim_counts;
	while(getline(an_inputstream, line, '\n')) {
		if ('>' == line[0]) {
			++delim_counts['>'];
			if(delim_counts['>'] > 10) {
				return FASTA;
			}
		}
		if ('@' == line[0]) {
			++delim_counts['@'];
			if(delim_counts['@'] > 10) {
				return FASTQ;
			}
		}
	}
//	vector<string> lines;
//	string line;
//	while (getline(an_inputstream, line, '\n')) {
//		lines.push_back(line);
//		if (lines.size() == 4) {
//			break;
//		}
//	}
//	if (lines.size() < 2) {
//		return NON_SEQUENCING;
//	}
//
//	if ('>' == lines[0][0]) {
//		char c = lines[1][0];
//		if ('A' == c || 'C' == c || 'G' == c || 'T' == c) {
//			return FASTA;
//		}
//	}
//	if ('@' == lines[0][0] && '+' == lines[2][0]) {
//		return FASTQ;
//	}
	return NON_SEQUENCING;
}

void IOUtils::save_FASTQ_with_trimming(const string& a_path) {
	string read_id_file = a_path + ".readid";
	string read_file = a_path + ".read";
	string qual_file = a_path + ".qual";
	bfs::path read_id_path(read_id_file);
	if (bfs::exists(read_id_path)) {
		return;
	}

	ifstream an_input_stream(a_path);
	string line;
	uint64_t current_sequence_size = 0;
	uint64_t current_quality_size = 0;
	uint64_t state = 0;
	uint64_t a_local_total_processed_bytes = 0;
	uint64_t a_local_processed_bytes = 0;

	uint64_t a_local_processed_lines = 0;
	uint64_t a_local_processed_reads = 0;

	stringstream s_read_id;
	stringstream s_read;
	stringstream s_qual;
	stringstream s_read_concatenating;
	stringstream s_qual_concatenating;
	ofstream out_read_id(read_id_file, ios::binary);
	ofstream out_read(read_file, ios::binary);
	ofstream out_qual(qual_file, ios::binary);

	while (getline(an_input_stream, line, '\n')) {
		++a_local_processed_lines;
		a_local_total_processed_bytes += line.size() + 1;
		if ('@' == line[0] && current_sequence_size == current_quality_size) {
			s_read_id << line << "\n";
			state = 1;
			if (0 == current_sequence_size) {
				continue;
			}
			++a_local_processed_reads;
			if (current_sequence_size == current_quality_size) {
				string a_read = s_read_concatenating.str();
				replace_unknown_bases(a_read);
				string a_qual = s_qual_concatenating.str();
				uint32_t base_start_id = 0;
				int32_t base_end_id = a_read.size() - 1;
				char c = a_read[base_start_id];
				if ('N' == c) {
					for (; base_start_id < a_read.size(); ++base_start_id) {
						c = a_read[base_start_id];
						if ('N' != c) {
							break;
						}
					}
				}
				c = a_read[a_read.size() - 1];
				if ('N' == c) {
					for (; base_end_id >= 0; --base_end_id) {
						c = a_read[base_end_id];
						if ('N' != c) {
							break;
						}
					}
				}
				++base_end_id;
				int32_t new_read_length = base_end_id - base_start_id;
				// some read contains only 'N' bases
				if (0 >= new_read_length) {
					base_start_id = 0;
					base_end_id = 1;
					new_read_length = 1;
				}
				s_read.write(&a_read[base_start_id], new_read_length);
				s_qual.write(&a_qual[base_start_id], new_read_length);
				a_local_processed_bytes += (new_read_length << 1) + 2;
				s_read << "\n";
				s_qual << "\n";
				if (a_local_processed_bytes > READ_BUFFER_SIZE) {
					out_read_id << s_read_id.rdbuf();
					out_read << s_read.rdbuf();
					out_qual << s_qual.rdbuf();
					s_read_id.str(string());
					s_read.str(string());
					s_qual.str(string());
					a_local_processed_bytes = 0;
				}
				s_read_concatenating.str(string());
				s_qual_concatenating.str(string());
			}
			current_sequence_size = 0;
			current_quality_size = 0;
		} else if ('+' == line[0] && 1 == state) {
			state = 2;
		} else if (1 == state) {
			current_sequence_size += line.size();
			s_read_concatenating << line;
		} else if (2 == state) {
			current_quality_size += line.size();
			s_qual_concatenating << line;
		}
	}
	// the final read
	++a_local_processed_reads;
	current_quality_size += line.size();
	s_qual_concatenating << line;

	if (current_sequence_size == current_quality_size) {
		s_read << s_read_concatenating.rdbuf();
		s_qual << s_qual_concatenating.rdbuf();
		s_read << "\n";
		s_qual << "\n";
		out_read_id << s_read_id.rdbuf();
		out_read << s_read.rdbuf();
		out_qual << s_qual.rdbuf();
	}
}

// Do not call this function within a thread since this function is already parallelized.
void IOUtils::find_skip_points(vector<uint64_t>& skip_points, const string& a_path, const uint64_t chunk_size, const uint64_t max_index,
        const uint64_t n_blocks, const uint64_t n_cores) {
	skip_points[0] = 0;
	skip_points[n_blocks] = max_index;

	vector<function<void()> > tasks;
	for (uint32_t id = 0; id < n_blocks - 1; ++id) {
		tasks.push_back([&, id] {
			ifstream input_stream(a_path, ios::binary);
			string line;
			const int64_t end_index = get_next_end_pos(id, n_blocks - 1, chunk_size, max_index, 0);
			input_stream.seekg(end_index, ios::beg);
			getline(input_stream, line, '\n');
			skip_points[id + 1] = (end_index + line.size() + 1);
		});
	}
	ParallelRunner::run_step_wise(n_cores, tasks, ParallelRunner::empty_ranged_func);
}

void IOUtils::find_fastq_skip_points(vector<uint64_t>& skip_points, const string& a_path, const uint64_t chunk_size, const uint64_t max_index,
        const uint64_t n_blocks, const uint64_t n_cores) {
	skip_points[0] = 0;
	skip_points[n_blocks] = max_index;

	vector<function<void()> > tasks;
	for (uint32_t id = 0; id < n_blocks - 1; ++id) {
		tasks.push_back([&, id] {
			ifstream input_stream(a_path, ios::binary);
			string line;
			const int64_t end_index = get_next_end_pos(id, n_blocks - 1, chunk_size, max_index, 0);
			input_stream.seekg(end_index, ios::beg);
			int64_t passed = end_index;
			int64_t temp_passed = 0;
			// we need to check the correct starting point, some problematic quality values start with '@' symbol.
			while(getline(input_stream, line, '\n')) {
				if('@' == line[0]) {
					temp_passed = line.size() + 1;
					getline(input_stream, line, '\n');
					temp_passed += line.size() + 1;
					getline(input_stream, line, '\n');
					temp_passed += line.size() + 1;
					if('+' == line[0]) {
//						cout << id << "/" << line << "\n";
						break;
					}
					passed += temp_passed;
					continue;
				}
				passed += line.size() + 1;
			}
			skip_points[id + 1] = passed;
		});
	}
//	ParallelRunner::run_step_wise(n_cores, tasks, ParallelRunner::empty_ranged_func);
	ParallelRunner::run_unbalanced_load(n_cores, tasks);
}

void IOUtils::find_fasta_skip_points(vector<uint64_t>& skip_points, const string& a_path, const uint64_t chunk_size, const uint64_t max_index,
		        const uint64_t n_blocks, const uint64_t n_cores) {
	skip_points[0] = 0;
	skip_points[n_blocks] = max_index;

	vector<function<void()> > tasks;
	for (uint32_t id = 0; id < n_blocks - 1; ++id) {
		tasks.push_back([&, id] {
			ifstream input_stream(a_path, ios::binary);
			string line;
			const int64_t end_index = get_next_end_pos(id, n_blocks - 1, chunk_size, max_index, 0);
			input_stream.seekg(end_index, ios::beg);
			int64_t passed = end_index;
//			int64_t temp_passed = 0;
			// we need to check the correct starting point, some problematic quality values start with '@' symbol.
			while(getline(input_stream, line, '\n')) {
				if('>' == line[0]) {
					break;
				}
				passed += line.size() + 1;
			}
			skip_points[id + 1] = passed;
		});
	}
//	ParallelRunner::run_step_wise(n_cores, tasks, ParallelRunner::empty_ranged_func);
	ParallelRunner::run_unbalanced_load(n_cores, tasks);
}
int64_t IOUtils::get_trimmed_indexes(int64_t& base_start_id, int64_t& base_end_id, const string& a_read) {
	char c = a_read[base_start_id];
	if ('N' == c) {
		int64_t max_index = a_read.size();
		for (; base_start_id < max_index; ++base_start_id) {
			c = a_read[base_start_id];
			if ('N' != c) {
				break;
			}
		}
	}
	c = a_read[base_end_id];
	if ('N' == c) {
		for (; base_end_id >= 0; --base_end_id) {
			c = a_read[base_end_id];
			if ('N' != c) {
				break;
			}
		}
	}
	++base_end_id;
	int64_t new_read_length = base_end_id - base_start_id;
	// some read contains only 'N' bases
	if (0 >= new_read_length) {
		base_start_id = 0;
		base_end_id = 1;
		new_read_length = 1;
	}
	return new_read_length;
}
void IOUtils::replace_unknown_bases(string& a_read) {
	u_int8_t a_char;
	// Convert unknown bases into 'N' bases.
	for (u_int32_t pos = 0; pos < a_read.size(); ++pos) {
		a_char = a_read[pos];
		a_read[pos] = DecodingMap[EncodingMap[a_char]];
	}
}
string IOUtils::read_fully(const string& a_path) {
	ifstream t(a_path);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	string buffer(size, ' ');
	t.seekg(0);
	t.read(&buffer[0], size);
	return buffer;
}

FASTQ_Info IOUtils::read_fastq(const string& a_path, bool verbose) {
	// For Sanger scores, they are lower than ':', but for illumina 1.3+, they are higher than ':'.
	bitset<255> illumina_bits;
	bitset<255> sanger_bits;
	for (auto itr = illumina_asciis.begin(); illumina_asciis.end() != itr; ++itr) {
		illumina_bits[*itr] = true;
	}
	for (auto itr = sanger_asciis.begin(); sanger_asciis.end() != itr; ++itr) {
		sanger_bits[*itr] = true;
	}
	Quality_Standard quality_standard = UNKNOWN;

	ifstream an_input_stream(a_path);
	string line;
	uint64_t current_sequence_size = 0;
	uint64_t current_quality_size = 0;
	uint64_t state = 0;
	uint64_t a_local_total_processed_bytes = 0;
	uint64_t a_local_processed_lines = 0;
	uint64_t a_local_processed_reads = 0;

	stringstream s_read;
	stringstream s_qual;
	stringstream s_read_concatenating;
	stringstream s_qual_concatenating;
	uint8_t max_quality = 0;
	uint8_t min_quality = 255;
	uint8_t q;
	bool found_sanger = false;
//	bool found_illumina = false;
	FASTQ_Info info;
	while (getline(an_input_stream, line, '\n')) {
		++a_local_processed_lines;
		a_local_total_processed_bytes += line.size() + 1;
		if ('@' == line[0] && current_sequence_size == current_quality_size) {
			state = 1;
			if (0 == current_sequence_size) {
				continue;
			}
			++a_local_processed_reads;
			if (current_sequence_size == current_quality_size) {
				string a_read = s_read_concatenating.str();
				string a_qual = s_qual_concatenating.str();
				uint32_t base_start_id = 0;
				int32_t base_end_id = a_read.size() - 1;
				char c = a_read[base_start_id];
				if ('N' == c) {
					for (; base_start_id < a_read.size(); ++base_start_id) {
						c = a_read[base_start_id];
						if ('N' != c) {
							break;
						}
					}
				}
				c = a_read[a_read.size() - 1];
				if ('N' == c) {
					for (; base_end_id >= 0; --base_end_id) {
						c = a_read[base_end_id];
						if ('N' != c) {
							break;
						}
					}
				}
				++base_end_id;
				int32_t new_read_length = base_end_id - base_start_id;
				// some read contains only 'N' bases
				if (0 >= new_read_length) {
					a_read = "N";
					a_qual = min_quality;
					base_start_id = 0;
					base_end_id = 1;
					new_read_length = 1;
				}
				s_read.write(&a_read[base_start_id], new_read_length);
				s_qual.write(&a_qual[base_start_id], new_read_length);
				s_read << "\n";
				s_qual << "\n";
				s_read_concatenating.str(string());
				s_qual_concatenating.str(string());
			}
			current_sequence_size = 0;
			current_quality_size = 0;
		} else if ('+' == line[0] && 1 == state) {
			state = 2;
		} else if (1 == state) {
			current_sequence_size += line.size();
			s_read_concatenating << line;
		} else if (2 == state) {
			current_quality_size += line.size();
			if (!found_sanger) {
				for (uint64_t qual_index = 0; qual_index < line.size(); ++qual_index) {
					q = line[qual_index];
					if (q < min_quality) {
						min_quality = q;
					}
					if (q > max_quality) {
						max_quality = q;
					}
					if (sanger_bits[q]) {
						found_sanger = true;
					}
				}
			}
			s_qual_concatenating << line;
		}
	}
	// the final read
	++a_local_processed_reads;
	current_quality_size += line.size();
	s_qual_concatenating << line;

	if (current_sequence_size == current_quality_size) {
//		cout << (boost::format("%s(%d)\n%s(%d)\n") %  s_read_concatenating.str() % current_sequence_size
//					% s_qual_concatenating.str() % current_quality_size).str();
		s_read << s_read_concatenating.rdbuf();
		s_qual << s_qual_concatenating.rdbuf();
		s_read << "\n";
		s_qual << "\n";
	}
	quality_standard = ILLUMINA;
	if (found_sanger) {
		quality_standard = SANGER;
	}
	info.read = s_read.str();
	info.qual = s_qual.str();
	info.standard = quality_standard;
	info.min_quality = min_quality;
	info.max_quality = max_quality;
	if (verbose) {
		cout
		        << (boost::format("[IOUtils.read_fastq] %s - Read: %d, Qual: %d, Processed bytes: %d\n") % a_path % info.read.size()
		                % info.qual.size() % a_local_total_processed_bytes).str();
	}
	return info;
}

Quality_Standard IOUtils::get_quality_standard(const string& a_path) {
	uint8_t max_quality = 0;
	uint8_t min_quality = 'h';
	// For Sanger scores, they are lower than ':', but for illumina 1.3+, they are higher than ':'.
	bitset<255> illumina_bits;
	bitset<255> sanger_bits;
	for (auto itr = illumina_asciis.begin(); illumina_asciis.end() != itr; ++itr) {
		illumina_bits[*itr] = true;
	}
	for (auto itr = sanger_asciis.begin(); sanger_asciis.end() != itr; ++itr) {
		sanger_bits[*itr] = true;
	}
	Quality_Standard quality_standard = UNKNOWN;
	ifstream paired_input_stream(a_path, ios::binary);
	uint8_t c = 64;
	string a_line;
	uint64_t current_sequence_size = 0;
	uint64_t current_quality_size = 0;
	uint64_t state = 0;
//	bool found_illumina = false;
	bool found_sanger = false;
	bool found = false;
	while (getline(paired_input_stream, a_line, '\n')) {
		if (found) {
			break;
		}
		if ('@' == a_line[0] && current_sequence_size == current_quality_size) {
			state = 1;
			if (0 == current_sequence_size) {
				continue;
			}
			current_sequence_size = 0;
			current_quality_size = 0;
		} else if ('+' == a_line[0] && 1 == state) {
			state = 2;
		} else if (1 == state) {
			current_sequence_size += a_line.size();
		} else if (2 == state) {
			current_quality_size += a_line.size();
			for (uint64_t i = 0; i < a_line.size(); ++i) {
				c = a_line[i];
				if (c > max_quality) {
					max_quality = c;
				} else if (c < min_quality) {
					min_quality = c;
				}
				if (illumina_bits[c]) {
//					found_illumina = true;
					found = true;
					break;
				}
				if (sanger_bits[c]) {
					found_sanger = true;
					found = true;
					break;
				}
			}
		}
	}

//	if(found_sanger) {
//		max_quality = 'I';
//		min_quality = '#';
//	} else {
//		max_quality = 'h';
//		min_quality = 'B';
//	}
//	if (min_quality > decision_value) {
//		quality_standard = ILLUMINA;
//	} else {
//		quality_standard = SANGER;
//	}
	quality_standard = ILLUMINA;
	if (found_sanger) {
		quality_standard = SANGER;
	}
	return quality_standard;
}

FASTQ_Info IOUtils::get_min_max_quality(const string& a_content) {
	bitset<255> illumina_bits;
	bitset<255> sanger_bits;
	for (auto itr = illumina_asciis.begin(); illumina_asciis.end() != itr; ++itr) {
		illumina_bits[*itr] = true;
	}
	for (auto itr = sanger_asciis.begin(); sanger_asciis.end() != itr; ++itr) {
		sanger_bits[*itr] = true;
	}
	Quality_Standard quality_standard = UNKNOWN;
	uint8_t min_quality = 'h';
	uint8_t max_quality = 0;
	uint8_t a_qual;
	bool found_sanger;
	bool found_illumina;
	for (uint64_t qual_index = 0; qual_index < a_content.size(); ++qual_index) {
		a_qual = a_content[qual_index];
		if ('\n' == a_qual) {
			continue;
		}
		if (sanger_bits[a_qual]) {
			found_sanger = true;
		}
		if (illumina_bits[a_qual]) {
			found_illumina = true;
		}
		if (a_qual > max_quality) {
			max_quality = a_qual;
		}
		if (a_qual < min_quality) {
			min_quality = a_qual;
		}
	}

	FASTQ_Info info;
	info.min_quality = min_quality;
	info.max_quality = max_quality;
	if (found_sanger) {
		quality_standard = SANGER;
	} else if (found_illumina) {
		quality_standard = ILLUMINA;
	}
	info.standard = quality_standard;
	return info;
}

// dealing with long-length reads
string IOUtils::read_fasta(const string& a_path) {
	ifstream an_input_stream(a_path);
	string line;
	string next_line;
	stringstream ss_read;
	while (getline(an_input_stream, line, '\n')) {
		if ('>' == line[0]) {
			continue;
		}
		ss_read << line << "\n";
	}
	return ss_read.str();
}
// we do not check dangling symbolic link
size_t IOUtils::get_file_size(const string& a_file_path) {
	if (!bfs::exists(a_file_path)) {
		return 0;
	}
	return bfs::file_size(a_file_path);
//	ifstream t(a_path);
//	t.seekg(0, std::ios::end);
//	return t.tellg();
}

void IOUtils::remove(const string& a_path) {
	if (!bfs::exists(a_path)) {
		return;
	}
	bfs::remove(a_path);
}

void IOUtils::remove_if_empty(const string& a_path) {
	if(0 != IOUtils::get_file_size(a_path))
		return;
	bfs::remove(a_path);
}

uint64_t IOUtils::get_number_of_lines(const string& a_path) {
	ifstream ifs(a_path, ios::in | ios::binary);
	uint64_t count = 0;
	vector<char> buffer(READ_BUFFER_SIZE);
	while (uint64_t actual_size_of_buffer = read_to_buffer(ifs, buffer)) {
		count += count_line_feeds(buffer, actual_size_of_buffer);
	}
	return count;
}
void IOUtils::combine_intermediate_files(const std::string& file_suffix, const int64_t max_block_id) {
	ofstream out(file_suffix);
	for(int64_t block_id = 0; block_id < max_block_id; ++block_id) {
		string output_read_path(file_suffix);
		output_read_path += "." + boost::lexical_cast<string>(block_id);
		if (boost::filesystem::exists(output_read_path)) {
			out << read_fully(output_read_path);
		}
	}
}
void IOUtils::remove_intermediate_files(const std::string& file_suffix, const int64_t max_block_id, const int64_t n_cores) {
	vector<function<void()> > tasks;
	for(int64_t block_id = 0; block_id < max_block_id; ++block_id) {
		tasks.push_back([&, block_id] {
			string output_read_path(file_suffix);
			output_read_path += "." + boost::lexical_cast<string>(block_id);
			if (boost::filesystem::exists(output_read_path)) {
				boost::filesystem::remove(output_read_path);
			}
		});
	}
	ParallelRunner::run_unbalanced_load(n_cores, tasks);
}
// Returns true if the filename has an extension indicating it is compressed
bool IOUtils::isGzip(const std::string& filename) {
	size_t suffix_length = GZIP_EXT.size() - 1;

	// Assume files without an extension are not compressed
	if (filename.size() < suffix_length)
		return false;

	std::string extension = filename.substr(filename.size() - suffix_length);
	return extension == GZIP_EXT;
}

// increase the soft limit to hard limit
void IOUtils::liftrlimit() {
#ifdef __linux__
	struct rlimit r;
	getrlimit(RLIMIT_AS, &r);
	if (r.rlim_cur < r.rlim_max)
		r.rlim_cur = r.rlim_max;
	setrlimit(RLIMIT_AS, &r);
#endif
}
} /* namespace castle */
