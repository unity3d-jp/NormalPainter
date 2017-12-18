using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace UTJ.BlendShapeEditor
{
    static class Utils
    {
        public static GameObject MeshToGameObject(Mesh mesh, Vector3 pos, Material[] materials = null)
        {
            if (materials == null)
                materials = new Material[1] { AssetDatabase.GetBuiltinExtraResource<Material>("Default-Diffuse.mat") };

            var go = new GameObject(mesh.name);
            var smr = go.AddComponent<SkinnedMeshRenderer>();
            smr.sharedMesh = mesh;
            smr.sharedMaterials = materials;
            go.GetComponent<Transform>().position = pos;
            return go;

        }

        public static Mesh ExtractMesh(UnityEngine.Object obj)
        {
            Mesh ret = null;
            var go = obj as GameObject;
            if (go != null)
            {
                var mf = go.GetComponent<MeshFilter>();
                if (mf != null)
                {
                    ret = mf.sharedMesh;
                }
                else
                {
                    var smr = go.GetComponent<SkinnedMeshRenderer>();
                    if (smr != null)
                        ret = smr.sharedMesh;
                }
            }
            else
            {
                ret = obj as Mesh;
            }
            return ret;
        }

        public static Material[] ExtractMaterials(UnityEngine.Object obj)
        {
            Material[] ret = null;
            var go = obj as GameObject;
            if (go != null)
            {
                var mf = go.GetComponent<MeshRenderer>();
                if (mf != null)
                {
                    ret = mf.sharedMaterials;
                }
                else
                {
                    var smr = go.GetComponent<SkinnedMeshRenderer>();
                    if (smr != null)
                        ret = smr.sharedMaterials;
                }
            }
            else
            {
                var mat = obj as Material;
                if(mat != null)
                    ret = new Material[1] { mat };
            }
            return ret;
        }
    }
}
