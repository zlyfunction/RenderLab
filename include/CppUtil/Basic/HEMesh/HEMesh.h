#pragma once
#ifndef _CPPUTIL_BASIC_HEMESH_BASE_HEMESH_H_
#define _CPPUTIL_BASIC_HEMESH_BASE_HEMESH_H_

#include <CppUtil/Basic/HEMesh/THalfEdge.h>
#include <CppUtil/Basic/HEMesh/TVertex.h>
#include <CppUtil/Basic/HEMesh/TEdge.h>
#include <CppUtil/Basic/HEMesh/TPolygon.h>

#include <CppUtil/Basic/random_set.h>

#include <assert.h>
#include <set>
#include <string>
#include <algorithm>

namespace CppUtil {
	namespace Basic {
		// nullptr Polygon is a boundary
		template<typename V,
			typename = std::enable_if_t<std::is_base_of_v<TVertex<V, typename V::E_t, typename V::P_t>, V>>,
			typename = std::enable_if_t<std::is_base_of_v<TEdge<V, typename V::E_t, typename V::P_t>, typename V::E_t>>,
			typename = std::enable_if_t<std::is_base_of_v<TPolygon<V, typename V::E_t, typename V::P_t>, typename V::P_t>>
		>
		class HEMesh : public HeapObj {
		public:
			using HE = typename V::HE;
			using E = typename V::E_t;
			using P = typename V::P_t;

		public:
			static const Ptr<HEMesh> New() { return Basic::New<HEMesh>(); }

		public:
			const std::vector<Ptr<V>> & Vertices() { return vertices.vec(); }
			const std::vector<Ptr<HE>> & HalfEdges() { return halfEdges.vec(); }
			const std::vector<Ptr<E>> & Edges() { return edges.vec(); }
			const std::vector<Ptr<P>> & Polygons() { return polygons.vec(); }

			size_t NumVertices() const { return vertices.size(); }
			size_t NumEdges() const { return edges.size(); }
			size_t NumPolygons() const { return polygons.size(); }
			size_t NumHalfEdges() const { return halfEdges.size(); }

			size_t Index(Ptr<V> v) const { return vertices.idx(v); }
			size_t Index(Ptr<E> e) const { return edges.idx(e); }
			size_t Index(Ptr<P> p) const { return polygons.idx(p); }
			const std::vector<size_t> Indices(Ptr<P> p) const;

			bool HaveBoundary() const;
			const std::vector<std::vector<Ptr<HE>>> Boundaries();
			size_t NumBoundaries() const { return const_cast<HEMesh*>(this)->Boundaries().size(); }
			bool IsTriMesh() const;
			// vertices empty => halfedges, edges and polygons empty
			bool IsEmpty() const { return vertices.empty(); }

			// min is 0
			bool Init(std::vector<std::vector<size_t>> polygons);
			bool Init(std::vector<size_t> polygons, size_t sides);
			void Clear();
			void Reserve(size_t n);
			const std::vector<std::vector<size_t>> Export() const;

			// -----------------
			//  basic mesh edit
			// -----------------

			template<typename ...Args>
			const Ptr<V> AddVertex(Args&& ... args) { return NewVertex(std::forward<Args>(args)...); }
			// e's halfedge is form v0 to v1
			template<typename ...Args>
			const Ptr<E> AddEdge(Ptr<V> v0, Ptr<V> v1, Args&& ... args);
			// polygon's halfedge is heLoop[0]
			template<typename ...Args>
			const Ptr<P> AddPolygon(const std::vector<Ptr<HE>> heLoop, Args&& ... args);
			void RemovePolygon(Ptr<P> polygon);
			void RemoveEdge(Ptr<E> e);
			void RemoveVertex(Ptr<V> v);

			// ----------------------
			//  high-level mesh edit
			// ----------------------

			// delete e
			const Ptr<V> SpiltEdge(Ptr<E> e);

			// counter-clock, remain e in container, won't break iteration
			bool RotateEdge(Ptr<E> e);

			// RemoveVertex and AddPolygon
			const Ptr<P> EraseVertex(Ptr<V> v);

			const Ptr<V> CollapseEdge(Ptr<E> e);

			// edge's halfedge : v0=>v1
			// nweV's halfedge : newV => v1
			template<typename ...Args>
			const Ptr<V> AddEdgeVertex(Ptr<E> e);

			// add a vertex in he.polygon
			// he.origin => new vertex => he.origin
			// polygon can be boundary
			template<typename ...Args>
			const Ptr<V> AddPolygonVertex(Ptr<HE> he, Args&& ... args);
			// add a vertex in polygon
			// polygon can be boundary
			template<typename ...Args>
			const Ptr<V> AddPolygonVertex(Ptr<P> p, Ptr<V> v, Args&& ... args);

			// connect he0.origin and he1.origin in he0/he1.polygon
			// [require] he0.polygon == he1.polygon, he0.origin != he1.origin
			// [return] edge with halfedge form he0.origin to he1.origin
			const Ptr<E> ConnectVertex(Ptr<HE> he0, Ptr<HE> he1);
			// connect v0 and v1 in p
			// call ConnectVertex(p->HalfEdge()->NextAt(v0), p->HalfEdge()->NextAt(v1))
			// [require] p isn't boundary(nullptr)
			const Ptr<E> ConnectVertex(Ptr<P> p, Ptr<V> v0, Ptr<V> v1) { return ConnectVertex(p->HalfEdge()->NextAt(v0), p->HalfEdge()->NextAt(v1)); }

			bool IsValid() const;
			bool HaveIsolatedVertices() const;

		private:
			// new and insert
			const Ptr<HE> NewHalfEdge();
			template<typename ...Args>
			const Ptr<V> NewVertex(Args&& ... args);
			template<typename ...Args>
			const Ptr<E> NewEdge(Args&& ... args);
			template<typename ...Args>
			const Ptr<P> NewPolygon(Args&& ... args);

			// clear and erase
			void DeleteHalfEdge(Ptr<HE> he) { he->Clear(); halfEdges.erase(he); }
			void DeleteVertex(Ptr<V> v) { v->Clear(); vertices.erase(v); }
			void DeleteEdge(Ptr<E> e) { e->Clear(); edges.erase(e); }
			void DeletePolygon(Ptr<P> p) { p->Clear(); polygons.erase(p); }
		protected:
			virtual ~HEMesh() = default;

		private:
			random_set<Ptr<HE>> halfEdges;
			random_set<Ptr<V>> vertices;
			random_set<Ptr<E>> edges;
			random_set<Ptr<P>> polygons;
		};

#include<CppUtil/Basic/HEMesh/HEMesh.inl>
	}
}

#endif // !_CPPUTIL_BASIC_HEMESH_BASE_HEMESH_H_
