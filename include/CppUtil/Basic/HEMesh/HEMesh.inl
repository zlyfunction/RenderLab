template<typename V, typename _0, typename _1, typename _2>
const std::vector<size_t> HEMesh<V, _0, _1, _2>::Indices(Ptr<P> p) const {
	std::vector<size_t> indices;
	for (auto v : p->BoundaryVertice())
		indices.push_back(Index(v));
	return indices;
}

template<typename V, typename _0, typename _1, typename _2>
const Ptr<typename HEMesh<V, _0, _1, _2>::HE> HEMesh<V, _0, _1, _2>::NewHalfEdge() {
	auto he = Basic::New<HE>();
	halfEdges.insert(he);
	return he;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<V> HEMesh<V, _0, _1, _2>::NewVertex(Args&& ... args) {
	auto v = Basic::New<V>(std::forward<Args>(args)...);
	vertices.insert(v);
	return v;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<typename V::E_t> HEMesh<V, _0, _1, _2>::NewEdge(Args&& ... args) {
	auto e = Basic::New<E>(std::forward<Args>(args)...);
	edges.insert(e);
	return e;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<typename V::P_t> HEMesh<V, _0, _1, _2>::NewPolygon(Args&& ... args) {
	auto p = Basic::New<P>(std::forward<Args>(args)...);
	polygons.insert(p);
	return p;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<typename V::E_t> HEMesh<V, _0, _1, _2>::AddEdge(Ptr<V> v0, Ptr<V> v1, Args && ... args) {
	if (v0 == v1) {
		printf("ERROR::HEMesh::AddEdge\n"
			"\t""v0 == v1\n");
		return nullptr;
	}
	if (V::IsConnected(v0, v1)) {
		printf("ERROR::HEMesh::AddEdge\n"
			"\t""v0 is already connected with v1\n");
		return nullptr;
	}

	auto e = NewEdge(std::forward<Args>(args)...);
	
	auto he0 = NewHalfEdge();
	auto he1 = NewHalfEdge();
	// [init]
	e->SetHalfEdge(he0);

	he0->SetNext(he1);
	he0->SetPair(he1);
	he0->SetOrigin(v0);
	he0->SetEdge(e);

	he1->SetNext(he0);
	he1->SetPair(he0);
	he1->SetOrigin(v1);
	he1->SetEdge(e);

	// [link he0]
	if (!v0->IsIsolated()) {
		auto inV0 = v0->FindFreeIncident();
		if (inV0 == nullptr) {
			printf("ERROR::HEMesh::AddEdge\n"
				"\t""v0 hasn't free incident\n");
			return nullptr;
		}
		auto outV0 = inV0->Next();

		inV0->SetNext(he0);
		he1->SetNext(outV0);
	}
	else
		v0->SetHalfEdge(he0);

	// [link he1]
	if (!v1->IsIsolated()) {
		auto inV1 = v1->FindFreeIncident();
		if (inV1 == nullptr) {
			printf("ERROR::HEMesh::AddEdge\n"
				"\t""v1 hasn't free incident\n");
			return nullptr;
		}
		auto outV1 = inV1->Next();

		inV1->SetNext(he1);
		he0->SetNext(outV1);
	}
	else
		v1->SetHalfEdge(he1);

	return e;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<typename V::P_t> HEMesh<V, _0, _1, _2>::AddPolygon(const std::vector<Ptr<HE>> heLoop, Args && ... args) {
	if (heLoop.size() == 0) {
		printf("ERROR::HEMesh::AddPolygon:\n"
			"\t""heLoop is empty\n");
		return nullptr;
	}
	for (int i = 0; i < heLoop.size(); i++) {
		if (!heLoop[i]->IsFree()) {
			printf("ERROR::HEMesh::AddPolygon:\n"
				"\t""heLoop[%d] isn't free\n", i);
			return nullptr;
		}
		int next = (i + 1) % heLoop.size();
		if (heLoop[i]->End() != heLoop[next]->Origin()) {
			printf("ERROR::HEMesh::AddPolygon:\n"
				"\t""heLoop[%d]'s end isn't heLoop[%d]'s origin\n", i, next);
			return nullptr;
		}
	}

	// reorder link
	for (int i = 0; i < heLoop.size(); i++) {
		int next = (i + 1) % heLoop.size();
		if (!HE::MakeAdjacent(heLoop[i], heLoop[next])) {
			printf("ERROR::HEMesh::AddPolygon:\n"
				"\t""the polygon would introduce a non-monifold condition\n");
			return nullptr;
		}
	}

	// link polygon and heLoop
	auto polygon = NewPolygon(std::forward<Args>(args)...);

	polygon->SetHalfEdge(heLoop[0]);
	for (auto he : heLoop)
		he->SetPolygon(polygon);

	return polygon;
}

template<typename V, typename _0, typename _1, typename _2>
void HEMesh<V, _0, _1, _2>::RemovePolygon(Ptr<P> polygon) {
	assert(polygon != nullptr);
	for (auto he : polygon->BoundaryHEs())
		he->SetPolygon(nullptr);
	DeletePolygon(polygon);
}

template<typename V, typename _0, typename _1, typename _2>
void HEMesh<V, _0, _1, _2>::RemoveEdge(Ptr<E> e) {
	assert(e != nullptr);
	auto he0 = e->HalfEdge();
	auto he1 = he0->Pair();

	if (!he0->IsFree())
		RemovePolygon(he0->Polygon());
	if (!he1->IsFree())
		RemovePolygon(he1->Polygon());

	// init
	auto v0 = he0->Origin();
	auto v1 = he1->Origin();
	auto inV0 = he0->Pre();
	auto inV1 = he1->Pre();
	auto outV0 = he0->RotateNext();
	auto outV1 = he1->RotateNext();

	// [link off he0]
	if (v0->HalfEdge() == he0)
		v0->SetHalfEdge(outV0 == he0 ? nullptr : outV0);

	inV0->SetNext(outV0);

	// [link off he1]
	if (v1->HalfEdge() == he1)
		v1->SetHalfEdge(outV1 == he1 ? nullptr : outV1);

	inV1->SetNext(outV1);

	// delete
	DeleteHalfEdge(he0);
	DeleteHalfEdge(he1);
	DeleteEdge(e);
}

template<typename V, typename _0, typename _1, typename _2>
void HEMesh<V, _0, _1, _2>::RemoveVertex(Ptr<V> v) {
	for (auto e : v->AdjEdges())
		RemoveEdge(e);
	DeleteVertex(v);
}

template<typename V, typename _0, typename _1, typename _2>
const Ptr<V> HEMesh<V, _0, _1, _2>::SpiltEdge(Ptr<E> e) {
	auto he01 = e->HalfEdge();
	auto he10 = he01->Pair();

	if (he01->IsBoundary() && he10->IsBoundary()) {
		printf("ERROR::HEMesh::SpiltEdge:\n"
			"\t""two side of edge are boundaries\n");
		return nullptr;
	}

	if (he01->IsBoundary() || he10->IsBoundary()) {
		if (he01->IsBoundary())
			std::swap(he01, he10);

		// he01 isn't boundary, he10 is boundary

		if (he01->Polygon()->Degree() != 3) {
			printf("ERROR::HEMesh::SpiltEdge:\n"
				"\t""polygon's degree %zd is not 3\n", he01->Polygon()->Degree());
			return nullptr;
		}

		e->SetHalfEdge(he01);
		auto v3 = AddEdgeVertex(e);
		auto he31 = v3->HalfEdge();
		auto he20 = he31->Next()->Next();
		ConnectVertex(he31, he20);
		return v3;
	}

	if (he01->Polygon()->Degree() != 3 || he10->Polygon()->Degree() != 3)
	{
		printf("ERROR::HEMesh::SpiltEdge:\n"
			"\t""polygon's degree (%zd, %zd) is not 3\n",
			he01->Polygon()->Degree(), he10->Polygon()->Degree());
		return nullptr;
	}

	auto v4 = AddEdgeVertex(e);
	auto he41 = v4->HalfEdge();
	auto he40 = he41->Pair()->Next();
	auto he20 = he41->Next()->Next();
	auto he31 = he40->Next()->Next();
	ConnectVertex(he41, he20);
	ConnectVertex(he40, he31);

	return v4;
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::RotateEdge(Ptr<E> e) {
	if (e->IsBoundary()) {
		printf("ERROR::HEMesh::RotateEdge:\n"
			"\t""e is boundary\n");
		return false;
	}
	// 1. prepare
	auto he01 = e->HalfEdge();
	auto he10 = he01->Pair();
	auto he02 = he10->Next();
	auto he13 = he01->Next();
	auto he01Pre = he01->Pre();
	auto he10Pre = he10->Pre();
	auto he02Next = he02->Next();
	auto he13Next = he13->Next();

	auto p01 = he01->Polygon();
	auto p10 = he10->Polygon();

	auto v0 = he01->Origin();
	auto v1 = he01->End();
	auto v2 = he02->End();
	auto v3 = he13->End();
	
	// 2. change
	if (v0->HalfEdge() == he01)
		v0->SetHalfEdge(he02);
	if (v1->HalfEdge() == he10)
		v1->SetHalfEdge(he13);

	auto he23 = he01;
	auto he32 = he10;

	he01Pre->SetNext(he02);

	he02->SetNext(he23);
	he02->SetPolygon(p01);

	he23->SetOrigin(v2);
	he32->SetOrigin(v3);

	he23->SetNext(he13Next);

	he10Pre->SetNext(he13);

	he13->SetNext(he32);
	he13->SetPolygon(p10);

	he32->SetNext(he02Next);

	if (p01->HalfEdge() == he13)
		p01->SetHalfEdge(he02);
	if (p10->HalfEdge() == he02)
		p10->SetHalfEdge(he13);

	return true;
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::Init(std::vector<std::vector<size_t>> polygons) {
	Clear();

	if (polygons.empty()) {
		printf("WARNNING::HEMesh::Init\n"
			"\t""polygons is empty\n");
		return false;
	}
	
	size_t max = 0;
	size_t min = SIZE_MAX;
	for (const auto& polygon : polygons) {
		if (polygon.size() <= 2) {
			printf("ERROR::HEMesh::Init\n"
				"\t""polygon's size <= 2\n");
			return false;
		}

		for (auto idx : polygon) {
			if (idx > max)
				max = idx;
			if (idx < min)
				min = idx;
		}
	}

	if (min != 0) {
		printf("ERROR::HEMesh::Init\n"
			"\t""min idx != 0\n");
		return false;
	}

	for (size_t i = 0; i <= max; i++)
		NewVertex();

	for (auto polygon : polygons) {
		vector<Ptr<HE>> heLoop;
		for (size_t i = 0; i < polygon.size(); i++) {
			size_t next = (i + 1) % polygon.size();
			if (polygon[i] == polygon[next]) {
				printf("WARNNING::HEMesh::Init\n"
					"\t""same idx (%zd)\n", polygon[i]);
				continue;
			}
			auto u = vertices[polygon[i]];
			auto v = vertices[polygon[next]];
			auto he = V::FindHalfEdge(u, v);
			if (!he)
				he = AddEdge(u, v)->HalfEdge();
			heLoop.push_back(he);
		}
		auto p = AddPolygon(heLoop);

		if (p == nullptr) {
			string polygonStr;
			for (auto idx : polygon)
				polygonStr += to_string(idx) + ", ";
			printf("WARNNING::HEMesh::Init\n"
				"\t""AddPolygon fail (%s)\n", polygonStr.c_str());
		}
	}

	return true;
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::Init(std::vector<size_t> polygons, size_t sides) {
	if (polygons.size() % sides != 0) {
		printf("ERROR::HEMesh::Init:\n"
		"\t""polygons.size() isn't an integer multiple of sides\n")
	}
	std::vector<std::vector<size_t>> arrangedPolygons;
	for (size_t i = 0; i < polygons.size(); i += sides) {
		arrangedPolygons.emplace_back();
		for (size_t j = 0; j < sides; j++)
			arrangedPolygons.back().push_back(polygons[i + j]);
	}
	return Init(arrangedPolygons);
}

template<typename V, typename _0, typename _1, typename _2>
const std::vector<std::vector<size_t>> HEMesh<V, _0, _1, _2>::Export() const {
	std::vector<std::vector<size_t>> arrangedPolygons;
	if (!IsValid())
		return arrangedPolygons;
	for (auto polygon : polygons.vec()) {
		arrangedPolygons.emplace_back();
		for (auto v : polygon->BoundaryVertice())
			arrangedPolygons.back().push_back(Index(v));
	}
	return arrangedPolygons;
}

template<typename V, typename _0, typename _1, typename _2>
void HEMesh<V, _0, _1, _2>::Clear() {
	vertices.clear();
	halfEdges.clear();
	edges.clear();
	polygons.clear();
}

template<typename V, typename _0, typename _1, typename _2>
void HEMesh<V, _0, _1, _2>::Reserve(size_t n) {
	vertices.reserve(n);
	halfEdges.reserve(6*n);
	edges.reserve(3*n);
	polygons.reserve(2*n);
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::HaveBoundary() const {
	for (auto he : halfEdges){
		if (he->IsBoundary())
			return true;
	}
	return false;
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::IsTriMesh() const {
	for (auto poly : polygons) {
		if (poly->Degree() != 3)
			return false;
	}
	return IsValid();
}

template<typename V, typename _0, typename _1, typename _2>
const std::vector<std::vector<Ptr<typename V::HE>>> HEMesh<V, _0, _1, _2>::Boundaries() {
	std::vector<std::vector<Ptr<HE>>> boundaries;
	std::set<Ptr<HE>> found;
	for (auto he : halfEdges) {
		if (he->IsBoundary() && found.find(he) == found.end()) {
			boundaries.push_back(std::vector<Ptr<HE>>());
			auto cur = he;
			do {
				boundaries.back().push_back(cur);
				found.insert(cur);
				cur = cur->Next();
			} while (cur != he);
		}
	}
	return boundaries;
}

template<typename V, typename _0, typename _1, typename _2>
const Ptr<typename V::P_t> HEMesh<V, _0, _1, _2>::EraseVertex(Ptr<V> v) {
	if (v->IsBoundary()) {
		RemoveVertex(v);
		return nullptr;
	}

	Ptr<HE> he = v->HalfEdge();
	while (he->Next()->End() == v) {
		he = he->RotateNext();
		if (he == v->HalfEdge()) {
			RemoveVertex(v);
			return nullptr;
		}
	}
	he = he->Next();

	RemoveVertex(v);
	return AddPolygon(he->Loop());
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<V> HEMesh<V, _0, _1, _2>::AddEdgeVertex(Ptr<E> e) {
	// prepare
	auto he01 = e->HalfEdge();
	auto he10 = he01->Pair();

	auto v0 = he01->Origin();
	auto v1 = he10->Origin();

	auto v0d = v0->Degree();
	auto v1d = v1->Degree();

	auto p01 = he01->Polygon();
	auto p10 = he10->Polygon();

	auto he02 = NewHalfEdge();
	auto he21 = NewHalfEdge();
	auto he12 = NewHalfEdge();
	auto he20 = NewHalfEdge();

	auto e02 = NewEdge();
	auto e12 = NewEdge();

	auto v2 = NewVertex(std::forward<Args>(args)...);

	// basic set
	if (v0->HalfEdge() == he01)
		v0->SetHalfEdge(he02);
	if (v1->HalfEdge() == he10)
		v1->SetHalfEdge(he12);
	v2->SetHalfEdge(he21);

	he02->SetNext(he21);
	he02->SetPair(he20);
	he02->SetOrigin(v0);
	he02->SetEdge(e02);
	he02->SetPolygon(p01);

	//he21->SetNext();
	he21->SetPair(he12);
	he21->SetOrigin(v2);
	he21->SetEdge(e12);
	he21->SetPolygon(p01);

	he12->SetNext(he20);
	he12->SetPair(he21);
	he12->SetOrigin(v1);
	he12->SetEdge(e12);
	he12->SetPolygon(p10);

	//he20->SetNext();
	he20->SetPair(he02);
	he20->SetOrigin(v2);
	he20->SetEdge(e02);
	he20->SetPolygon(p10);

	e02->SetHalfEdge(he02);
	e12->SetHalfEdge(he12);

	if (!P::IsBoundary(p01) && p01->HalfEdge() == he01)
		p01->SetHalfEdge(he02);
	if (!P::IsBoundary(p10) && p10->HalfEdge() == he10)
		p10->SetHalfEdge(he12);

	// 4 case
	if (v0d == 1 && v1d == 1) {
		he21->SetNext(he12);
		he20->SetNext(he02);
	}
	else if (v0d == 1) {
		auto he01Next = he01->Next();
		auto he10Pre = he10->Pre();
		
		he21->SetNext(he01Next);
		he20->SetNext(he02);
		he10Pre->SetNext(he12);
	}
	else if (v1d == 1) {
		auto he01Pre = he01->Pre();
		auto he10Next = he10->Next();

		he01Pre->SetNext(he02);
		he21->SetNext(he12);
		he20->SetNext(he10Next);
	}
	else {
		auto he01Pre = he01->Pre();
		auto he01Next = he01->Next();
		auto he10Pre = he10->Pre();
		auto he10Next = he10->Next();

		he01Pre->SetNext(he02);
		he10Pre->SetNext(he12);
		he21->SetNext(he01Next);
		he20->SetNext(he10Next);
	}

	// delete
	DeleteHalfEdge(he01);
	DeleteHalfEdge(he10);
	DeleteEdge(e);

	return v2;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<V> HEMesh<V, _0, _1, _2>::AddPolygonVertex(Ptr<HE> he, Args && ... args) {
	auto p = he->Polygon();

	auto pre = he->Pre();
	auto v0 = he->Origin();
	auto v1 = AddVertex(std::forward<Args>(args)...);
	
	auto he01 = NewHalfEdge();
	auto he10 = NewHalfEdge();
	auto e01 = NewEdge();

	he01->SetNext(he10);
	he01->SetPair(he10);
	he01->SetOrigin(v0);
	he01->SetEdge(e01);
	he01->SetPolygon(p);

	he10->SetNext(he);
	he10->SetPair(he01);
	he10->SetOrigin(v1);
	he10->SetEdge(e01);
	he10->SetPolygon(p);

	e01->SetHalfEdge(he01);
	v1->SetHalfEdge(he10);

	pre->SetNext(he01);

	return v1;
}

template<typename V, typename _0, typename _1, typename _2>
template<typename ...Args>
const Ptr<V> HEMesh<V, _0, _1, _2>::AddPolygonVertex(Ptr<P> p, Ptr<V> v, Args&& ... args) {
	Ptr<HE> he;
	if (P::IsBoundary(p)) {
		for (auto outHE : v->AdjOutHEs()) {
			if (P::IsBoundary(outHE->Polygon())) {
				he = outHE;
				break;
			}
		}
		if (he == nullptr) {
			printf("ERROR::HEMesh::AddPolygonVertex:\n"
				"\t""p is boundary(nullptr) but v is not on boundary\n");
			return nullptr;
		}
	}
	else
		he = p->HalfEdge()->NextAt(v);
	
	return AddPolygonVertex(he, std::forward<Args>(args)...);
}

template<typename V, typename _0, typename _1, typename _2>
const Ptr<typename V::E_t> HEMesh<V, _0, _1, _2>::ConnectVertex(Ptr<HE> he0, Ptr<HE> he1) {
	auto p = he0->Polygon();
	if (p != he1->Polygon()) {
		printf("ERROR::HEMesh::ConnectVertex:\n"
			"\t""he0->Polygon != he1->Polygon\n");
		return nullptr;
	}

	if (P::IsBoundary(p)) {
		printf("ERROR::HEMesh::ConnectVertex:\n"
			"\t""halfedge's polygon is boundary\n");
		return nullptr;
	}

	auto v0 = he0->Origin();
	auto v1 = he1->Origin();
	if (v0 == v1) {
		printf("ERROR::HEMesh::ConnectVertex:\n"
			"\t""he0->Origin == he1->Origin\n");
		return nullptr;
	}

	if (V::IsConnected(v0, v1)) {
		printf("ERROR::HEMesh::ConnectVertex:\n"
			"\t""v0 and v1 is already connected\n");
		return nullptr;
	}

	RemovePolygon(p);

	auto he0Pre = he0->Pre();
	auto he1Pre = he1->Pre();

	auto he0Loop = he0->NextTo(he1);
	auto he1Loop = he1->NextTo(he0);
	
	auto e01 = NewEdge();
	auto he01 = NewHalfEdge();
	auto he10 = NewHalfEdge();

	e01->SetHalfEdge(he01);

	he01->SetNext(he1);
	he01->SetPair(he10);
	he01->SetOrigin(v0);
	he01->SetEdge(e01);
	
	he10->SetNext(he0);
	he10->SetPair(he01);
	he10->SetOrigin(v1);
	he10->SetEdge(e01);

	he0Pre->SetNext(he01);
	he1Pre->SetNext(he10);

	he0Loop.push_back(he10);
	he1Loop.push_back(he01);

	AddPolygon(he0Loop);
	AddPolygon(he1Loop);

	return e01;
}

template<typename V, typename _0, typename _1, typename _2>
const Ptr<V> HEMesh<V, _0, _1, _2>::CollapseEdge(Ptr<E> e) {
	auto he01 = e->HalfEdge();
	auto he10 = he01->Pair();

	auto v0 = he01->Origin();
	auto v1 = he01->End();

	std::vector<Ptr<V>> comV;
	auto v0AdjVs = v0->AdjVertices();
	auto v1AdjVs = v1->AdjVertices();
	std::set_intersection(v0AdjVs.begin(), v0AdjVs.end(), v1AdjVs.begin(), v1AdjVs.end(),
		std::insert_iterator<std::vector<Ptr<V>>>(comV, comV.begin()));
	
	if (comV.size() != 2)
	{
#if !NDEBUG
		printf("WARNNING::HEMesh::CollapseEdge:\n"
			"\t""N(v0) �� N(v1) != 2\n");
#endif
		return nullptr;
	}

	auto p01 = he01->Polygon();
	auto p10 = he10->Polygon();

	if (v0->Degree() == 1) {
		EraseVertex(v0);
		return v1;
	}
	if (v1->Degree() == 1) {
		EraseVertex(v1);
		return v0;
	}

	// set v
	auto v = NewVertex();
	v->SetHalfEdge(he01->Pre()->Pair());

	for (auto he : he01->RotateNext()->RotateNextTo(he01))
		he->SetOrigin(v);
	for (auto he : he10->RotateNext()->RotateNextTo(he10))
		he->SetOrigin(v);

	if (he01->Loop().size() == 3) { // p01->Degree() == 3
		auto he01Next = he01->Next();
		auto he01Pre = he01->Pre();
		auto he01NextPair = he01Next->Pair();
		auto he01PrePair = he01Pre->Pair();

		auto v2 = he01Pre->Origin();
		if (v2->HalfEdge() == he01Pre)
			v2->SetHalfEdge(he01NextPair);

		auto newE = NewEdge();

		he01NextPair->SetPair(he01PrePair);
		he01NextPair->SetEdge(newE);
		he01PrePair->SetPair(he01NextPair);
		he01PrePair->SetEdge(newE);
		newE->SetHalfEdge(he01NextPair);

		DeleteEdge(he01Next->Edge());
		DeleteEdge(he01Pre->Edge());
		DeleteHalfEdge(he01Next);
		DeleteHalfEdge(he01Pre);
		if (!P::IsBoundary(p01))
			DeletePolygon(p01);
	}
	else { //p01->Degree >= 4
		if (!P::IsBoundary(p01) && p01->HalfEdge() == he01)
			p01->SetHalfEdge(he01->Next());
		he01->Pre()->SetNext(he01->Next());
	}

	if (he10->Loop().size() == 3) { // p10->Degree() == 3
		auto he10Next = he10->Next();
		auto he10Pre = he10->Pre();
		auto he10NextPair = he10Next->Pair();
		auto he10PrePair = he10Pre->Pair();

		auto v2 = he10Pre->Origin();
		if (v2->HalfEdge() == he10Pre)
			v2->SetHalfEdge(he10NextPair);

		auto newE = NewEdge();

		he10NextPair->SetPair(he10PrePair);
		he10NextPair->SetEdge(newE);
		he10PrePair->SetPair(he10NextPair);
		he10PrePair->SetEdge(newE);
		newE->SetHalfEdge(he10NextPair);

		DeleteEdge(he10Next->Edge());
		DeleteEdge(he10Pre->Edge());
		DeleteHalfEdge(he10Next);
		DeleteHalfEdge(he10Pre);
		if (!P::IsBoundary(p10))
			DeletePolygon(p10);
	}
	else { //p10->Degree >= 4
		if (!P::IsBoundary(p10) && p10->HalfEdge() == he10)
			p10->SetHalfEdge(he10->Next());
		he10->Pre()->SetNext(he10->Next());
	}

	DeleteVertex(v0);
	DeleteVertex(v1);
	DeleteHalfEdge(he01);
	DeleteHalfEdge(he10);
	DeleteEdge(e);

	return v;
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::IsValid() const {
	for (auto he : halfEdges) {
		if (!he->Next() || !he->Pair() || !he->Origin() || !he->Edge())
			return false;
	}
	set<Ptr<HE>> uncheckHEs(halfEdges.begin(), halfEdges.end());
	Ptr<HE> headHE;
	Ptr<HE> curHE;
	while (!uncheckHEs.empty() || headHE != nullptr) {
		if (!headHE) {
			auto iter = uncheckHEs.begin();
			headHE = curHE = *iter;
			uncheckHEs.erase(iter);
		}
		curHE = curHE->Next();
		if (curHE == headHE)
			headHE = nullptr;
		else {
			auto target = uncheckHEs.find(curHE);
			if (target == uncheckHEs.end())
				return false;
			uncheckHEs.erase(target);
		}
	}

	for (auto he : halfEdges) {
		if (he->Next()->Origin() != he->End() && he->Next()->Origin() != he->Origin())
			return false;
		if (he->Pair()->Pair() != he)
			return false;
	}
	for (auto v : vertices) {
		for (auto he : v->AdjOutHEs()) {
			if (he->Origin() != v)
				return false;
		}
	}
	for (auto e : edges) {
		if (!e->HalfEdge() || e->HalfEdge()->Edge() != e || e->HalfEdge()->Pair()->Edge() != e)
			return false;
	}
	for (auto p : polygons) {
		if (!p->HalfEdge())
			return false;
		for (auto he : p->BoundaryHEs()) {
			if (he->Polygon() != p)
				return false;
		}
	}
	return true;
}

template<typename V, typename _0, typename _1, typename _2>
bool HEMesh<V, _0, _1, _2>::HaveIsolatedVertices() const {
	for (auto v : vertices) {
		if (v->IsIsolated())
			return true;
	}
	return false;
}
