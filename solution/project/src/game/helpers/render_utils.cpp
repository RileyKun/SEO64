#include "render_utils.hpp"

#include "input.hpp"
#include "../../app/features/notifs/notifs.hpp"

void render_utils::RenderSphere(const Vector &vCenter, float flRadius, int nTheta, int nPhi, Color c, bool bZBuffer)
{
	IMaterial *const mat{ bZBuffer ? *s::s_pVertexColor.cast<IMaterial **>() : *s::s_pVertexColorIgnoreZ.cast<IMaterial **>() };

	RenderSphere(vCenter, flRadius, nTheta, nPhi, c, mat);
}

void render_utils::RenderSphere(const Vector &vCenter, float flRadius, int nTheta, int nPhi, Color c, IMaterial *pMaterial)
{
	s::RenderSphere.call<void>(vCenter, flRadius, nTheta, nPhi, c, pMaterial);
}

void render_utils::RenderWireframeSphere(const Vector &vCenter, float flRadius, int nTheta, int nPhi, Color c, bool bZBuffer)
{
	IMaterial *const mat{ bZBuffer ? *s::s_pWireframe.cast<IMaterial **>() : *s::s_pWireframeIgnoreZ.cast<IMaterial **>() };

	RenderSphere(vCenter, flRadius, nTheta, nPhi, c, mat);
}

void render_utils::RenderWireframeBox(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Color c, bool bZBuffer)
{
	s::RenderWireframeBox.call<void>(vOrigin, angles, vMins, vMaxs, c, bZBuffer);
}

void render_utils::RenderWireframeSweptBox(const Vector &vStart, const Vector &vEnd, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Color c, bool bZBuffer)
{
	s::RenderWireframeSweptBox.call<void>(vStart, vEnd, angles, vMins, vMaxs, c, bZBuffer);
}

void render_utils::RenderBox(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Color c, IMaterial *pMaterial, bool bInsideOut)
{
	s::RenderBoxMaterial.call<void>(vOrigin, angles, vMins, vMaxs, c, pMaterial, bInsideOut);
}

void render_utils::RenderBox(const Vector &origin, const QAngle &angles, const Vector &mins, const Vector &maxs, Color c, bool bZBuffer, bool bInsideOut)
{
	s::RenderBox.call<void>(origin, angles, mins, maxs, c, bZBuffer, bInsideOut);
}

void render_utils::RenderLine(const Vector &v1, const Vector &v2, Color c, bool bZBuffer)
{
	s::RenderLine.call<void>(v1, v2, c, bZBuffer);
}

void render_utils::RenderTriangle(const Vector &p1, const Vector &p2, const Vector &p3, Color c, bool bZBuffer)
{
	s::RenderTriangle.call<void>(p1, p2, p3, c, bZBuffer);
}

void render_utils::RenderTriangle(const Vector &p1, const Vector &p2, const Vector &p3, Color c, IMaterial *pMaterial)
{
	s::RenderTriangle_Material.call<void>(p1, p2, p3, c, pMaterial);
}

void render_utils::GenerateBoxVertices(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Vector pVerts[8])
{
	s::GenerateBoxVertices.call<void>(vOrigin, angles, vMins, vMaxs, pVerts);
}

void render_utils::RenderAxes(const Vector &vOrigin, float flScale, bool bZBuffer)
{
	const vec3 x_end{ vOrigin + vec3{ flScale, 0.0f, 0.0f } };
	RenderLine(vOrigin, x_end, Color{ 255, 0, 0, 255 }, bZBuffer);

	const vec3 y_end{ vOrigin + vec3{ 0.0f, flScale, 0.0f } };
	RenderLine(vOrigin, y_end, Color{ 0, 255, 0, 255 }, bZBuffer);

	const vec3 z_end{ vOrigin + vec3{ 0.0f, 0.0f, flScale } };
	RenderLine(vOrigin, z_end, Color{ 0, 0, 255, 255 }, bZBuffer);
}

void render_utils::RenderAxes(const matrix3x4_t &transform, float flScale, bool bZBuffer)
{
	vec3 x_axis{};
	vec3 y_axis{};
	vec3 z_axis{};

	math::matrixGetColumn(transform, 0, x_axis);
	math::matrixGetColumn(transform, 1, y_axis);
	math::matrixGetColumn(transform, 2, z_axis);

	vec3 origin{};

	math::matrixGetColumn(transform, 3, origin);

	const vec3 x_end{ origin + x_axis * flScale };
	const vec3 y_end{ origin + y_axis * flScale };
	const vec3 z_end{ origin + z_axis * flScale };

	RenderLine(origin, x_end, Color{ 255, 0, 0, 255 }, bZBuffer);
	RenderLine(origin, y_end, Color{ 0, 255, 0, 255 }, bZBuffer);
	RenderLine(origin, z_end, Color{ 0, 0, 255, 255 }, bZBuffer);
}

void render_utils::DrawAxes(const Vector &origin, Vector *pts, int idx, Color c, CMeshBuilder &meshBuilder)
{
	s::DrawAxes_MeshBuilder.call<void>(origin, pts, idx, c, meshBuilder);
}

void render_utils::DrawExtrusionFace(const Vector &start, const Vector &end, Vector *pts, int idx1, int idx2, Color c, CMeshBuilder &meshBuilder)
{
	s::DrawExtrustionFace.call<void>(start, end, pts, idx1, idx2, c, meshBuilder);
}

void render_utils::DrawScreenSpaceRectangle(IMaterial *pMaterial,
											int nDestX, int nDestY, int nWidth, int nHeight, float flSrcTextureX0, float flSrcTextureY0, float flSrcTextureX1, float flSrcTextureY1, int nSrcTextureWidth, int nSrcTextureHeight, void *pClientRenderable, int nXDice, int nYDice, float fDepth)
{
	s::DrawScreenSpaceRectangle.call<void>(pMaterial, nDestX, nDestY, nWidth, nHeight, flSrcTextureX0, flSrcTextureY0, flSrcTextureX1, flSrcTextureY1, nSrcTextureWidth, nSrcTextureHeight, pClientRenderable, nXDice, nYDice, fDepth);
}