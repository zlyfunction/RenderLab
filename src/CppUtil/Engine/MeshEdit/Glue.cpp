#include <CppUtil/Engine/MeshEdit/Glue.h>

#include <CppUtil/Engine/TriMesh.h>
#include <map>

using namespace CppUtil;
using namespace CppUtil::Basic;
using namespace CppUtil::Engine;
using namespace std;

bool Glue::Run() {
	if (triMesh == nullptr || triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::Glue::Run:\n"
			"\t""triMesh == nullptr || triMesh->GetType() == TriMesh::INVALID\n");
		return false;
	}

	auto & positions = triMesh->GetPositions();
	vector<uint> indice;
	vector<Point3> uniquePos;
	map<Point3, uint> pos2idx;

	for (auto triangle : triMesh->GetTriangles()) {
		for (int i = 0; i < 3; i++) {
			auto pos = positions[triangle->idx[i]];
			auto target = pos2idx.find(pos);
			if (target == pos2idx.end()) {
				pos2idx[pos] = static_cast<uint>(uniquePos.size());
				uniquePos.push_back(pos);
				target = pos2idx.find(pos);
			}
			indice.push_back(target->second);
		}
	}
	printf("Glue: %zd -> %zd\n", positions.size(), uniquePos.size());

	triMesh->Init(indice, uniquePos);

	return true;
}
