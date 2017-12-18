using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace UTJ.BlendShapeEditor
{
    [Serializable]
    public class BlendShapeFrameData
    {
        public float weight = 100.0f;
        public UnityEngine.Object mesh;
    }

    [Serializable]
    public class BlendShapeData
    {
        public bool fold = true;
        public string name = "";
        public List<BlendShapeFrameData> frames = new List<BlendShapeFrameData>();

        public void ClearInvalidFrames()
        {
            frames.RemoveAll(item => { return item.mesh == null; });
        }

        public void NormalizeWeights()
        {
            int n = frames.Count;
            float step = 100.0f / n;
            for (int i = 0; i < n; ++i)
            {
                frames[i].weight = step * (i + 1);
            }
        }
    }

    [Serializable]
    public class BlendShapeEditorData : ScriptableObject
    {
        public UnityEngine.Object baseMesh;
        public List<BlendShapeData> blendShapeData = new List<BlendShapeData>();
    }
}

