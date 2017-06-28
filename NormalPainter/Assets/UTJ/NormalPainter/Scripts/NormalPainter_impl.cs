using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ.NormalPainter
{
    public enum EditMode
    {
        Select,
        Brush,
        Assign,
        Move,
        Rotate,
        Scale,
        Smooth,
        Projection,
        Reset,
    }

    public enum BrushMode
    {
        Paint,
        Replace,
        Smooth,
        Reset,
    }

    public enum SelectMode
    {
        Single,
        Rect,
        Lasso,
        Brush,
    }

    public enum MirrorMode
    {
        None,
        RightToLeft,
        LeftToRight,
        ForwardToBack,
        BackToForward,
        UpToDown,
        DownToUp,
    }

    public enum ImageFormat
    {
        PNG,
        EXR,
    }

    public enum ModelOverlay
    {
        None,
        LocalSpaceNormals,
        TangentSpaceNormals,
        VertexColor,
    }

    public enum Coordinate
    {
        World,
        Local,
        Pivot,
    }

    public enum SceneGUIState
    {
        Repaint = 1 << 0,
        SelectionChanged = 1 << 1,
    }



    public partial class NormalPainter : MonoBehaviour
    {
#if UNITY_EDITOR

        public Vector3 ToWorldVector(Vector3 v, Coordinate c)
        {
            switch(c)
            {
                case Coordinate.Local: return GetComponent<Transform>().localToWorldMatrix.MultiplyVector(v);
                case Coordinate.Pivot: return m_settings.pivotRot * v;
            }
            return v;
        }


        public void ApplyAssign(Vector3 v, Coordinate c)
        {
            var trans = GetComponent<Transform>().localToWorldMatrix;
            v = ToWorldVector(v, c).normalized;

            npAssign(m_selection, m_points.Length, ref trans, v, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyMove(Vector3 v, Coordinate c)
        {
            var trans = GetComponent<Transform>().localToWorldMatrix;
            v = ToWorldVector(v, c);

            npMove(m_selection, m_points.Length, ref trans, v, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyRotate(Quaternion amount, Quaternion pivotRot, Coordinate c)
        {
            Matrix4x4 trans;
            var t = GetComponent<Transform>();
            switch (c)
            {
                case Coordinate.World:
                    trans = t.localToWorldMatrix;
                    pivotRot = Quaternion.identity;
                    break;
                case Coordinate.Local:
                    trans = Matrix4x4.identity;
                    pivotRot = Quaternion.identity;
                    break;
                case Coordinate.Pivot:
                    trans = t.localToWorldMatrix;
                    break;
                default: return;
            }

            npRotate(m_points, m_selection, m_points.Length, ref trans, amount, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyRotatePivot(Quaternion amount, Vector3 pivotPos, Quaternion pivotRot, Coordinate c)
        {
            Matrix4x4 trans;
            var t = GetComponent<Transform>();
            switch (c)
            {
                case Coordinate.World:
                    trans = t.localToWorldMatrix;
                    pivotRot = Quaternion.identity;
                    break;
                case Coordinate.Local:
                    trans = Matrix4x4.identity;
                    pivotPos = t.worldToLocalMatrix.MultiplyPoint(pivotPos);
                    pivotRot = Quaternion.identity;
                    break;
                case Coordinate.Pivot:
                    trans = t.localToWorldMatrix;
                    break;
                default: return;
            }

            npRotatePivot(m_points, m_selection, m_points.Length, ref trans, amount, pivotPos, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyScale(Vector3 amount, Vector3 pivotPos, Quaternion pivotRot, Coordinate c)
        {
            Matrix4x4 trans;
            var t = GetComponent<Transform>();
            switch (c)
            {
                case Coordinate.World:
                    trans = t.localToWorldMatrix;
                    pivotRot = Quaternion.identity;
                    break;
                case Coordinate.Local:
                    trans = Matrix4x4.identity;
                    pivotPos = t.worldToLocalMatrix.MultiplyPoint(pivotPos);
                    pivotRot = Quaternion.identity;
                    break;
                case Coordinate.Pivot:
                    trans = t.localToWorldMatrix;
                    break;
                default: return;
            }

            npScale(m_points, m_selection, m_points.Length, ref trans, amount, pivotPos, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public bool ApplyPaintBrush(bool useSelection, Vector3 pos, float radius, float strength, float[] bsamples, Vector3 baseDir, int blendMode)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (npBrushPaint(m_points, selection, m_points.Length, ref trans, pos, radius, strength, bsamples, bsamples.Length, baseDir, blendMode, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public bool ApplyReplaceBrush(bool useSelection, Vector3 pos, float radius, float strength, float[] bsamples, Vector3 amount)
        {
            amount = GetComponent<Transform>().worldToLocalMatrix.MultiplyVector(amount).normalized;

            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (npBrushReplace(m_points, selection, m_points.Length, ref trans, pos, radius, strength, bsamples, bsamples.Length, amount, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public bool ApplySmoothBrush(bool useSelection, Vector3 pos, float radius, float strength, float[] bsamples)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (npBrushSmooth(m_points, selection, m_points.Length, ref trans, pos, radius, strength, bsamples, bsamples.Length, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public bool ApplyResetBrush(bool useSelection, Vector3 pos, float radius, float strength, float[] bsamples)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (npBrushLerp(m_points, selection, m_points.Length, ref trans, pos, radius, strength, bsamples, bsamples.Length, m_normalsBase, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public void ResetNormals(bool useSelection)
        {
            if (!useSelection)
            {
                Array.Copy(m_normalsBase, m_normals, m_normals.Length);
            }
            else
            {
                for (int i = 0; i < m_normals.Length; ++i)
                    m_normals[i] = Vector3.Lerp(m_normals[i], m_normalsBase[i], m_selection[i]).normalized;
            }
            UpdateNormals();
        }

        public void PushUndo()
        {
            Undo.RecordObject(this, "NormalEditor");
            m_history.count++;
            if (m_history.normals != null && m_history.normals.Length == m_normals.Length)
                Array.Copy(m_normals, m_history.normals, m_normals.Length);
            else
                m_history.normals = (Vector3[])m_normals.Clone();
            Undo.FlushUndoRecordObjects();
            //Debug.Log("PushUndo(): " + m_history.count);
        }

        public void OnUndoRedo()
        {
            //Debug.Log("OnUndoRedo(): " + m_history.count);
            UpdateSkinning();
            if (m_history.normals != null && m_normals != null && m_history.normals.Length == m_normals.Length)
            {
                Array.Copy(m_history.normals, m_normals, m_normals.Length);
                UpdateNormals();
            }
        }


        bool UpdateBoneMatrices()
        {
            if (!m_skinned) { return false; }

            bool ret = false;
            var smr = GetComponent<SkinnedMeshRenderer>();

            var rootMatrix = GetComponent<Transform>().localToWorldMatrix;
            if (m_rootMatrix != rootMatrix)
            {
                m_rootMatrix = rootMatrix;
                ret = true;
            }
            for (int i = 0; i < m_boneMatrices.Length; ++i)
            {
                if (m_boneMatrices[i] != smr.bones[i].localToWorldMatrix)
                {
                    m_boneMatrices[i] = smr.bones[i].localToWorldMatrix;
                    ret = true;
                }
            }
            return ret;
        }

        void UpdateSkinning()
        {
            if (UpdateBoneMatrices())
            {
                npApplySkinning(
                    m_meshTarget.boneWeights, ref m_rootMatrix, m_boneMatrices, m_meshTarget.bindposes, m_points.Length, m_boneMatrices.Length,
                    m_meshTarget.vertices, m_meshTarget.normals, m_meshTarget.tangents, m_points, m_normals, m_tangents);
                npApplySkinning(
                    m_meshTarget.boneWeights, ref m_rootMatrix, m_boneMatrices, m_meshTarget.bindposes, m_points.Length, m_boneMatrices.Length,
                    null, m_normalsBasePredeformed, m_tangentsBasePredeformed, null, m_normalsBase, m_tangentsBase);

                m_cbPoints.SetData(m_points);
                m_cbNormals.SetData(m_normals);
                m_cbBaseNormals.SetData(m_normalsBase);
                m_cbTangents.SetData(m_tangents);
                m_cbBaseTangents.SetData(m_tangentsBase);

                if(m_meshSkinBaked)
                    m_meshSkinBaked.Clear();
            }
        }

        public void UpdateNormals(bool upload = true)
        {
            if (m_cbNormals != null)
                m_cbNormals.SetData(m_normals);

            if (m_meshTarget != null)
            {
                if (m_skinned)
                {
                    UpdateBoneMatrices();
                    npApplyReverseSkinning(
                        m_meshTarget.boneWeights, ref m_rootMatrix, m_boneMatrices, m_meshTarget.bindposes, m_points.Length, m_boneMatrices.Length,
                        null, m_normals, null, null, m_normalsTmp, null);
                    m_meshTarget.normals = m_normalsTmp;
                }
                else
                {
                    m_meshTarget.normals = m_normals;
                }

                if (upload)
                    m_meshTarget.UploadMeshData(false);
            }
        }

        public void UpdateSelection()
        {
            int prevSelected = m_numSelected;

            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            m_numSelected = npUpdateSelection(m_points, m_normals, m_selection, m_points.Length, ref trans,
                ref m_selectionPos, ref m_selectionNormal);

            m_selectionRot = Quaternion.identity;
            if (m_numSelected > 0)
            {
                m_selectionRot = Quaternion.LookRotation(m_selectionNormal);
                m_settings.pivotPos = m_selectionPos;
                m_settings.pivotRot = m_selectionRot;
            }

            if(prevSelected == 0 && m_numSelected == 0)
            {
                // no need to upload
            }
            else
            {
                m_cbSelection.SetData(m_selection);
            }
        }

        public void RecalculateTangents()
        {
            m_meshTarget.RecalculateTangents();
            m_tangents = m_meshTarget.tangents;
            if(m_cbTangents == null)
                m_cbTangents = new ComputeBuffer(m_tangents.Length, 16);
            m_cbTangents.SetData(m_tangents);
        }


        public bool Raycast(Event e, ref Vector3 pos, ref int ti)
        {
            Ray ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);
            float d = 0.0f;
            if(Raycast(ray, ref ti, ref d))
            {
                pos = ray.origin + ray.direction * d;
                return true;
            }
            return false;
        }

        public bool Raycast(Ray ray, ref int ti, ref float distance)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            bool ret = npRaycast(ray.origin, ray.direction,
                m_points, m_triangles, m_triangles.Length / 3, ref ti, ref distance, ref trans) > 0;
            return ret;
        }

        public Vector3 PickNormal(Vector3 pos, int ti)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npPickNormal(m_points, m_triangles, m_normals, ref trans, pos, ti);
        }

        public Vector3 PickBaseNormal(Vector3 pos, int ti)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npPickNormal(m_points, m_triangles, m_normalsBase, ref trans, pos, ti);
        }


        public bool SelectEdge(float strength = 1.0f, bool clear = true)
        {
            return npSelectEdge(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength, clear) > 0;
        }

        public bool SelectConnected(float strength = 1.0f, bool clear = true)
        {
            return npSelectConnected(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength, clear) > 0;
        }

        public bool SelectAll()
        {
            for (int i = 0; i < m_selection.Length; ++i)
                m_selection[i] = 1.0f;
            return m_selection.Length > 0;
        }

        public bool ClearSelection()
        {
            System.Array.Clear(m_selection, 0, m_selection.Length);
            return m_selection.Length > 0;
        }

        public static Vector2 ScreenCoord11(Vector2 v)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            var pixelRect = cam.pixelRect;
            var rect = cam.rect;
            return new Vector2(
                    v.x / pixelRect.width * rect.width * 2.0f - 1.0f,
                    (v.y / pixelRect.height * rect.height * 2.0f - 1.0f) * -1.0f);
        }

        public bool SelectVertex(Event e, float strength, bool frontFaceOnly)
        {
            var center = e.mousePosition;
            var size = new Vector2(15.0f, 15.0f);
            var r1 = center - size;
            var r2 = center + size;
            return SelectVertex(r1, r2, strength, frontFaceOnly);
        }
        public bool SelectVertex(Vector2 r1, Vector2 r2, float strength, bool frontFaceOnly)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix * trans;
            r1 = ScreenCoord11(r1);
            r2 = ScreenCoord11(r2);
            var rmin = new Vector2(Math.Min(r1.x, r2.x), Math.Min(r1.y, r2.y));
            var rmax = new Vector2(Math.Max(r1.x, r2.x), Math.Max(r1.y, r2.y));

            return npSelectSingle(m_points, m_normalsBase, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, rmin, rmax, campos, frontFaceOnly) > 0;
        }

        public bool SelectTriangle(Event e, float strength)
        {
            Ray ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);
            return SelectTriangle(ray, strength);
        }
        public bool SelectTriangle(Ray ray, float strength)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npSelectTriangle(m_points, m_triangles, m_triangles.Length / 3, m_selection, strength, ref trans, ray.origin, ray.direction) > 0;
        }


        public bool SelectRect(Vector2 r1, Vector2 r2, float strength, bool frontFaceOnly)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix * trans;
            r1 = ScreenCoord11(r1);
            r2 = ScreenCoord11(r2);
            var rmin = new Vector2(Math.Min(r1.x, r2.x), Math.Min(r1.y, r2.y));
            var rmax = new Vector2(Math.Max(r1.x, r2.x), Math.Max(r1.y, r2.y));

            return npSelectRect(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, rmin, rmax, campos, frontFaceOnly) > 0;
        }

        public bool SelectLasso(Vector2[] points, float strength, bool frontFaceOnly)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix * trans;

            return npSelectLasso(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, points, points.Length, campos, frontFaceOnly) > 0;
        }

        public bool SelectBrush(Vector3 pos, float radius, float strength, float[] bsamples)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npSelectBrush(m_points, m_points.Length, ref trans, pos, radius, strength, bsamples, bsamples.Length, m_selection) > 0;
        }

        public static Vector3 GetMirrorPlane(MirrorMode mirrorMode)
        {
            switch (mirrorMode)
            {
                case MirrorMode.RightToLeft:    return Vector3.left;
                case MirrorMode.LeftToRight:    return Vector3.right;
                case MirrorMode.ForwardToBack:  return Vector3.back;
                case MirrorMode.BackToForward:  return Vector3.forward;
                case MirrorMode.UpToDown:       return Vector3.down;
                case MirrorMode.DownToUp:       return Vector3.up;
            }
            return Vector3.up;
        }

        MirrorMode m_prevMirrorMode;

        public bool ApplyMirroring()
        {
            if (m_settings.mirrorMode == MirrorMode.None) return false;

            bool needsSetup = false;
            if (m_mirrorRelation == null || m_mirrorRelation.Length != m_points.Length)
            {
                m_mirrorRelation = new int[m_points.Length];
                needsSetup = true;
            }
            else if(m_prevMirrorMode != m_settings.mirrorMode)
            {
                m_prevMirrorMode = m_settings.mirrorMode;
                needsSetup = true;
            }

            Vector3 planeNormal = GetMirrorPlane(m_settings.mirrorMode);
            if (needsSetup)
            {
                if (npBuildMirroringRelation(m_points, m_normalsBase, m_points.Length, planeNormal, 0.001f, m_mirrorRelation) == 0)
                {
                    Debug.LogWarning("NormalEditor: this mesh seems not symmetric");
                    m_mirrorRelation = null;
                    m_settings.mirrorMode = MirrorMode.None;
                    return false;
                }
            }
            npApplyMirroring(m_mirrorRelation, m_normals.Length, planeNormal, m_normals);
            return true;
        }


        static Rect FromToRect(Vector2 start, Vector2 end)
        {
            Rect r = new Rect(start.x, start.y, end.x - start.x, end.y - start.y);
            if (r.width < 0)
            {
                r.x += r.width;
                r.width = -r.width;
            }
            if (r.height < 0)
            {
                r.y += r.height;
                r.height = -r.height;
            }
            return r;
        }

        public bool BakeToTexture(int width, int height, string path)
        {
            if (path == null || path.Length == 0)
                return false;

            m_matBake.SetBuffer("_BaseNormals", m_cbBaseNormals);
            m_matBake.SetBuffer("_BaseTangents", m_cbBaseTangents);

            var rt = new RenderTexture(width, height, 0, RenderTextureFormat.ARGBHalf);
            var tex = new Texture2D(rt.width, rt.height, TextureFormat.RGBAHalf, false);
            rt.Create();

            m_cmdDraw.Clear();
            m_cmdDraw.SetRenderTarget(rt);
            for (int si = 0; si < m_meshTarget.subMeshCount; ++si)
                m_cmdDraw.DrawMesh(m_meshTarget, Matrix4x4.identity, m_matBake, si, 0);
            Graphics.ExecuteCommandBuffer(m_cmdDraw);

            RenderTexture.active = rt;
            tex.ReadPixels(new Rect(0, 0, tex.width, tex.height), 0, 0, false);
            tex.Apply();
            RenderTexture.active = null;

            if (path.EndsWith(".png"))
                System.IO.File.WriteAllBytes(path, tex.EncodeToPNG());
            else
                System.IO.File.WriteAllBytes(path, tex.EncodeToEXR());


            DestroyImmediate(tex);
            DestroyImmediate(rt);

            return true;
        }

        public bool BakeToVertexColor()
        {
            var color = new Color[m_normals.Length];
            for (int i = 0; i < m_normals.Length; ++i)
            {
                color[i].r = m_normals[i].x * 0.5f + 0.5f;
                color[i].g = m_normals[i].y * 0.5f + 0.5f;
                color[i].b = m_normals[i].z * 0.5f + 0.5f;
                color[i].a = 1.0f;
            }
            m_meshTarget.colors = color;
            return true;
        }

        public bool LoadTexture(Texture tex)
        {
            if (tex == null)
                return false;

            bool packed = false;
            {
                var path = AssetDatabase.GetAssetPath(tex);
                var importer = AssetImporter.GetAtPath(path) as TextureImporter;
                if (importer != null)
                    packed = importer.textureType == TextureImporterType.NormalMap;
            }

            var cbUV = new ComputeBuffer(m_normals.Length, 8);
            cbUV.SetData(m_meshTarget.uv);

            m_csBakeFromMap.SetInt("_Packed", packed ? 1 : 0);
            m_csBakeFromMap.SetTexture(0, "_NormalMap", tex);
            m_csBakeFromMap.SetBuffer(0, "_UV", cbUV);
            m_csBakeFromMap.SetBuffer(0, "_Normals", m_cbBaseNormals);
            m_csBakeFromMap.SetBuffer(0, "_Tangents", m_cbBaseTangents);
            m_csBakeFromMap.SetBuffer(0, "_Dst", m_cbNormals);
            m_csBakeFromMap.Dispatch(0, m_normals.Length, 1, 1);

            m_cbNormals.GetData(m_normals);
            cbUV.Dispose();

            UpdateNormals();
            PushUndo();

            return true;
        }

        public bool LoadVertexColor()
        {
            var color = m_meshTarget.colors;
            if (color.Length != m_normals.Length)
                return false;

            for (int i = 0; i < color.Length; ++i)
            {
                m_normals[i].x = color[i].r * 2.0f - 1.0f;
                m_normals[i].y = color[i].g * 2.0f - 1.0f;
                m_normals[i].z = color[i].b * 2.0f - 1.0f;
            }
            UpdateNormals();
            PushUndo();
            return true;
        }

        public void ApplySmoothing(float radius, float strength)
        {
            var selection = m_numSelected > 0 ? m_selection : null;
            var mat = GetComponent<Transform>().localToWorldMatrix;
            npSmooth(m_points, selection, m_points.Length, ref mat, radius, strength, m_normals);
            UpdateNormals();
        }

        public bool ApplyWelding()
        {
            if(npWeld(m_points, m_selection, m_points.Length, m_normals) > 0)
            {
                UpdateNormals();
                return true;
            }
            return false;
        }

        public void ApplyProjection(GameObject go, bool baseNormalsAsRayDirection)
        {
            if (go == null) { return; }

            Mesh mesh = null;
            {
                var mf = go.GetComponent<MeshFilter>();
                if (mf != null)
                    mesh = mf.sharedMesh;
                else
                {
                    var smi = go.GetComponent<SkinnedMeshRenderer>();
                    if (smi != null)
                    {
                        mesh = new Mesh();
                        smi.BakeMesh(mesh);
                    }
                }
            }
            if (mesh == null)
            {
                Debug.LogWarning("ProjectNormals(): projector has no mesh!");
                return;
            }

            var ptrans = go.GetComponent<Transform>().localToWorldMatrix;
            ApplyProjection(mesh, ptrans, baseNormalsAsRayDirection);
        }

        public void ApplyProjection(Mesh projector, Matrix4x4 ptrans, bool baseNormalsAsRayDirection)
        {
            var mat = GetComponent<Transform>().localToWorldMatrix;
            var ppoints = projector.vertices;
            var prtiangles = projector.triangles;
            var pnormals = projector.normals;
            var selection = m_numSelected > 0 ? m_selection : null;
            if (pnormals.Length == 0)
            {
                Debug.LogWarning("ProjectNormals(): projector mesh has no normals!");
                return;
            }

            var rayDirections = baseNormalsAsRayDirection ? m_normalsBase : m_normals;
            npProjectNormals(m_points, rayDirections, selection, m_points.Length, ref mat,
                ppoints, pnormals, prtiangles, prtiangles.Length / 3, ref ptrans, m_normals);
            UpdateNormals();
            PushUndo();
        }

        public void ResetToBindpose(bool pushUndo)
        {
            var smr = GetComponent<SkinnedMeshRenderer>();
            if (smr == null || smr.bones == null || smr.sharedMesh == null) { return; }

            var bones = smr.bones;
            var bindposes = smr.sharedMesh.bindposes;
            var bindposeMap = new Dictionary<Transform, Matrix4x4>();

            for (int i = 0; i < bones.Length; i++)
            {
                if (!bindposeMap.ContainsKey(bones[i]))
                {
                    bindposeMap.Add(bones[i], bindposes[i]);
                }
            }

            if (pushUndo)
                Undo.RecordObjects(bones, "NormalPainter: ResetToBindpose");

            foreach (var kvp in bindposeMap)
            {
                var bone = kvp.Key;
                var imatrix = kvp.Value;
                var localMatrix =
                    bindposeMap.ContainsKey(bone.parent) ? (imatrix * bindposeMap[bone.parent].inverse).inverse : imatrix.inverse;

                bone.localPosition = localMatrix.MultiplyPoint(Vector3.zero);
                bone.localRotation = Quaternion.LookRotation(localMatrix.GetColumn(2), localMatrix.GetColumn(1));
                bone.localScale = new Vector3(localMatrix.GetColumn(0).magnitude, localMatrix.GetColumn(1).magnitude, localMatrix.GetColumn(2).magnitude);
            }

            if (pushUndo)
                Undo.FlushUndoRecordObjects();
        }

        public void ExportSettings(string path)
        {
            AssetDatabase.DeleteAsset(path);
            AssetDatabase.CreateAsset(Instantiate(m_settings), path);
        }


        [DllImport("NormalPainterCore")] static extern int npRaycast(
            Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_triangles,
            ref int tindex, ref float distance, ref Matrix4x4 trans);

        [DllImport("NormalPainterCore")] static extern Vector3 npPickNormal(
            Vector3[] vertices, int[] indices, Vector3[] normals, ref Matrix4x4 trans,
            Vector3 pos, int ti);

        [DllImport("NormalPainterCore")] static extern int npSelectSingle(
            Vector3[] vertices, Vector3[] normals, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2 rmin, Vector2 rmax, Vector3 campos, bool frontfaceOnly);

        [DllImport("NormalPainterCore")] static extern int npSelectTriangle(
            Vector3[] vertices, int[] indices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 trans, Vector3 pos, Vector3 dir);
        
        [DllImport("NormalPainterCore")] static extern int npSelectEdge(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength, bool clear);

        [DllImport("NormalPainterCore")] static extern int npSelectConnected(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength, bool clear);

        [DllImport("NormalPainterCore")] static extern int npSelectRect(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2 rmin, Vector2 rmax, Vector3 campos, bool frontfaceOnly);

        [DllImport("NormalPainterCore")] static extern int npSelectLasso(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2[] points, int num_points, Vector3 campos, bool frontfaceOnly);
        
        [DllImport("NormalPainterCore")] static extern int npSelectBrush(
            Vector3[] vertices, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float[] bsamples, int num_bsamples, float[] seletion);

        [DllImport("NormalPainterCore")] static extern int npUpdateSelection(
            Vector3[] vertices, Vector3[] normals, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            ref Vector3 selection_pos, ref Vector3 selection_normal);


        [DllImport("NormalPainterCore")] static extern int npBrushReplace(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float[] bsamples, int num_bsamples, Vector3 amount, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npBrushPaint(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float[] bsamples, int num_bsamples, Vector3 baseNormal, int blend_mode, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npBrushSmooth(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float[] bsamples, int num_bsamples, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npBrushLerp(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float[] bsamples, int num_bsamples, Vector3[] baseNormals, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npAssign(
            float[] selection, int num_vertices, ref Matrix4x4 trans, Vector3 amount, Vector3[] normals);
        
        [DllImport("NormalPainterCore")] static extern int npMove(
            float[] selection, int num_vertices, ref Matrix4x4 trans, Vector3 amount, Vector3[] normals);
        
        [DllImport("NormalPainterCore")] static extern int npRotate(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Quaternion amount, Quaternion pivotRot, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npRotatePivot(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Quaternion amount, Vector3 pivotPos, Quaternion pivotRot, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npScale(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Vector3 amount, Vector3 pivotPos, Quaternion pivotRot, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npSmooth(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            float radius, float strength, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npWeld(
            Vector3[] vertices, float[] selection, int num_vertices, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern int npBuildMirroringRelation(
            Vector3[] vertices, Vector3[] base_normals, int num_vertices, Vector3 plane_normal, float epsilon, int[] relation);

        [DllImport("NormalPainterCore")] static extern void npApplyMirroring(
            int[] relation, int num_vertices, Vector3 plane_normal, Vector3[] normals);

        [DllImport("NormalPainterCore")] static extern void npProjectNormals(
            Vector3[] vertices, Vector3[] normals, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Vector3[] pvertices, Vector3[] pnormals, int[] pindices, int num_triangles, ref Matrix4x4 ptrans,
            Vector3[] dst);

        [DllImport("NormalPainterCore")] static extern void npApplySkinning(
            BoneWeight[] weights, ref Matrix4x4 root, Matrix4x4[] bones, Matrix4x4[] bindposes, int num_vertices, int num_bones,
            Vector3[] ipoints, Vector3[] inormals, Vector4[] itangents,
            Vector3[] opoints, Vector3[] onormals, Vector4[] otangents);

        [DllImport("NormalPainterCore")] static extern void npApplyReverseSkinning(
            BoneWeight[] weights, ref Matrix4x4 root, Matrix4x4[] bones, Matrix4x4[] bindposes, int num_vertices, int num_bones,
            Vector3[] ipoints, Vector3[] inormals, Vector4[] itangents,
            Vector3[] opoints, Vector3[] onormals, Vector4[] otangents);

        [DllImport("NormalPainterCore")] static extern void npInitializePenInput();
#endif
    }
}
