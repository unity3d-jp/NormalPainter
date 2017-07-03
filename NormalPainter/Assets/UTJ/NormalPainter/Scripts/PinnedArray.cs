using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace UTJ.NormalPainter
{

    public class PinnedObject<T> : IDisposable
    {
        T m_data;
        GCHandle m_gch;

        public PinnedObject(T data)
        {
            m_data = data;
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }

        public T Object { get { return m_data; } }
        public IntPtr Pointer { get { return m_gch.AddrOfPinnedObject(); } }


        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (m_gch.IsAllocated)
                    m_gch.Free();
            }
        }

        public static implicit operator IntPtr(PinnedObject<T> v) { return v.Pointer; }
        public static implicit operator T (PinnedObject<T> v) { return v.Object; }
    }


    public class PinnedArray<T> : IDisposable, IEnumerable<T>
    {
        T[] m_data;
        GCHandle m_gch;

        public PinnedArray(int size)
        {
            m_data = new T[size];
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }
        public PinnedArray(T[] data, bool clone = false)
        {
            m_data = clone ? (T[])data.Clone() : data;
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }

        public int Length { get { return m_data.Length; } }
        public T this[int i]
        {
            get { return m_data[i]; }
            set { m_data[i] = value; }
        }
        public T[] Array { get { return m_data; } }
        public IntPtr Pointer { get { return m_gch.AddrOfPinnedObject(); } }

        public PinnedArray<T> Clone() { return new PinnedArray<T>((T[])m_data.Clone()); }
        public bool Assign(T[] source)
        {
            if (source != null && m_data.Length == source.Length)
            {
                System.Array.Copy(source, m_data, m_data.Length);
                return true;
            }
            return false;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (m_gch.IsAllocated)
                    m_gch.Free();
            }
        }

        public IEnumerator<T> GetEnumerator()
        {
            return (IEnumerator<T>)m_data.GetEnumerator();
        }
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public static implicit operator IntPtr(PinnedArray<T> v) { return v.Pointer; }
        public static implicit operator T[](PinnedArray<T> v) { return v.Array; }
    }


    // Pinned"List" but assume size is fixed (== functionality is same as PinnedArray).
    // this class is intended to pass to Mesh.GetNormals(), Mesh.SetNormals(), and C++ functions.
    public class PinnedList<T> : IDisposable, IEnumerable<T>
    {
        List<T> m_list;
        T[] m_data;
        GCHandle m_gch;

        #region dirty
        static System.Reflection.FieldInfo s_itemsField;
        static System.Reflection.FieldInfo s_sizeField;

        static System.Reflection.FieldInfo GetItemsField()
        {
            if (s_itemsField == null)
            {
                s_itemsField = typeof(List<T>).GetField("_items",
                    System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
            }
            return s_itemsField;
        }
        static System.Reflection.FieldInfo GetSizeField()
        {
            if (s_sizeField == null)
            {
                s_sizeField = typeof(List<T>).GetField("_size",
                    System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
            }
            return s_sizeField;
        }

        public static T[] ListGetInternalArray(List<T> list)
        {
            return (T[])GetItemsField().GetValue(list);
        }
        public static List<T> ListCreateIntrusive(T[] data)
        {
            var ret = new List<T>();
            // force assign existing array to internal data
            GetItemsField().SetValue(ret, data);
            GetSizeField().SetValue(ret, data.Length);
            return ret;
        }
        #endregion


        public PinnedList(int size)
        {
            m_list = new List<T>(size);
            m_data = ListGetInternalArray(m_list);
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }
        public PinnedList(T[] data, bool clone = false)
        {
            if (clone)
            {
                m_list = new List<T>(data);
                m_data = ListGetInternalArray(m_list);
            }
            else
            {
                m_data = data;
                m_list = ListCreateIntrusive(m_data);
            }
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }
        public PinnedList(List<T> data, bool clone = false)
        {
            m_list = clone ? new List<T>(data) : data;
            m_data = ListGetInternalArray(m_list);
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }


        public int Length { get { return m_data.Length; } }
        public T this[int i]
        {
            get { return m_data[i]; }
            set { m_data[i] = value; }
        }
        public T[] Array { get { return m_data; } }
        public List<T> List { get { return m_list; } }
        public IntPtr Pointer { get { return m_gch.AddrOfPinnedObject(); } }

        public PinnedList<T> Clone() { return new PinnedList<T>(m_list, true); }
        public bool Assign(T[] source)
        {
            if (source != null && m_data.Length == source.Length)
            {
                System.Array.Copy(source, m_data, m_data.Length);
                return true;
            }
            return false;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (m_gch.IsAllocated)
                    m_gch.Free();
            }
        }

        public IEnumerator<T> GetEnumerator()
        {
            return (IEnumerator<T>)m_data.GetEnumerator();
        }
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public static implicit operator IntPtr(PinnedList<T> v) { return v.Pointer; }
        public static implicit operator T[] (PinnedList<T> v) { return v.Array; }
        public static implicit operator List<T> (PinnedList<T> v) { return v.List; }
    }

}
