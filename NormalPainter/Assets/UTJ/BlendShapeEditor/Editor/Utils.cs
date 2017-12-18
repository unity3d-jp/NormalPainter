using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace UTJ.BlendShapeEditor
{
    static class Utils
    {
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
                {
                    ret = new Material[1] { mat };
                }
            }
            return ret;
        }
    }
}
