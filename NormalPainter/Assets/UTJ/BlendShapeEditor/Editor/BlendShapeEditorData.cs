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
    }

    [Serializable]
    public class BlendShapeEditorData : ScriptableObject
    {
        public UnityEngine.Object baseMesh;
        public List<BlendShapeData> blendShapeData = new List<BlendShapeData>();
    }
}

