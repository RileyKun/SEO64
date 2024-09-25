#pragma once

#include "class_ids.hpp"

using CMeshBuilder = void *; // mfef: inline asm is the death of me, not adding this nonsense

namespace render_utils 
{
	void RenderSphere(const Vector &vCenter, float flRadius, int nTheta, int nPhi, Color c, bool bZBuffer);
	void RenderSphere(const Vector &vCenter, float flRadius, int nTheta, int nPhi, Color c, IMaterial *pMaterial);
	void RenderWireframeSphere(const Vector &vCenter, float flRadius, int nTheta, int nPhi, Color c, bool bZBuffer);
	void GenerateBoxVertices(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Vector pVerts[8]);
	void RenderAxes(const Vector &vOrigin, float flScale, bool bZBuffer);
	void RenderAxes(const matrix3x4_t &transform, float flScale, bool bZBuffer);
	void DrawAxes(const Vector &origin, Vector *pts, int idx, Color c, CMeshBuilder &meshBuilder);
	void DrawExtrusionFace(const Vector &start, const Vector &end, Vector *pts, int idx1, int idx2, Color c, CMeshBuilder &meshBuilder);
	void DrawScreenSpaceRectangle(IMaterial *pMaterial, int nDestX, int nDestY, int nWidth, int nHeight, float flSrcTextureX0, float flSrcTextureY0, float flSrcTextureX1, float flSrcTextureY1, int nSrcTextureWidth, int nSrcTextureHeight, void *pClientRenderable, int nXDice, int nYDice, float fDepth);
	void RenderWireframeBox(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Color c, bool bZBuffer);
	void RenderBox(const Vector &vOrigin, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Color c, IMaterial *pMaterial, bool bInsideOut);
	void RenderWireframeSweptBox(const Vector &vStart, const Vector &vEnd, const QAngle &angles, const Vector &vMins, const Vector &vMaxs, Color c, bool bZBuffer);
	void RenderBox(const Vector &origin, const QAngle &angles, const Vector &mins, const Vector &maxs, Color c, bool bZBuffer, bool bInsideOut = false);
	void RenderLine(const Vector &v1, const Vector &v2, Color c, bool bZBuffer);
	void RenderTriangle(const Vector &p1, const Vector &p2, const Vector &p3, Color c, bool bZBuffer);
	void RenderTriangle(const Vector &p1, const Vector &p2, const Vector &p3, Color c, IMaterial *pMaterial);
}