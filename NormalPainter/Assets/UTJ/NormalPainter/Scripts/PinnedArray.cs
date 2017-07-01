using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace UTJ.NormalPainter
{
    public class PinnedArray<T> : IDisposable, IEnumerable<T>
    {
        T[] m_data;
        GCHandle m_gch;

        public PinnedArray(int size)
        {
            m_data = new T[size];
            m_gch = GCHandle.Alloc(m_data, GCHandleType.Pinned);
        }
        public PinnedArray(T[] data, bool clone = true)
        {
            m_data = clone ? (T[])data.Clone() : m_data;
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

}
