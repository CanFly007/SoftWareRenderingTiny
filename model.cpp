#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) :verts_(), faces_()
{
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail())return;
	std::string line;
	while (!in.eof())
	{
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v "))
		{
			iss >> trash;
			Vec3f v;
			for (int i = 0; i < 3; i++)				
				iss >> v.raw[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vt "))
		{
			iss >> trash >> trash;
			Vec2f uv;
			iss >> uv.x;
			iss >> uv.y;
			//for (int i = 0; i < 2; i++)
			//	iss >> uv[i];
			uv_.push_back(uv);
		}
		else if (!line.compare(0, 2, "f "))
		{
			std::vector<int> f;
			int itrash, idx;
			iss >> trash;
			while (iss >> idx >> trash >> itrash >> trash >> itrash)
			{
				idx--;
				f.push_back(idx);
			}
			faces_.push_back(f);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << std::endl;
	load_texture(filename, "_diffuse.tga", diffusemap_);
}

Model::~Model() {}

int Model::nverts()
{
	return (int)verts_.size();
}
int Model::nfaces()
{
	return (int)faces_.size();
}
std::vector<int> Model::face(int idx)
{
	return faces_[idx];
}
Vec3f Model::vert(int i)
{
	return verts_[i];
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img)
{
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos)
	{
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading" << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}

TGAColor Model::diffuse(Vec2i uv)
{
	return diffusemap_.get(uv.x, uv.y);
}

Vec2i Model::uv(int iface, int nvert)
{
	std::vector<int> face = faces_[iface];
	int idx = face[nvert];
	//int idx = faces_[iface][nvert][1];
	return Vec2i(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}