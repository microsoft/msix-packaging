//-----------------------------------------------------------------------
// <copyright file="Block.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using System;
    using System.Runtime.InteropServices;
    using Microsoft.Msix.Utils.AppxPackagingInterop;

    /// <summary>
    /// Stores information about a block.
    /// </summary>
    public class Block
    {
        /// <summary>
        /// Initializes a new instance of the Block class.
        /// </summary>
        /// <param name="compressedSize">Compressed size of the block</param>
        /// <param name="hash">Block hash</param>
        /// <param name="index">Zero-based index of the block</param>
        private Block(ulong compressedSize, string hash, int index)
        {
            if (string.IsNullOrWhiteSpace(hash))
            {
                throw new ArgumentNullException("hash");
            }
            else if (index < 0)
            {
                throw new ArgumentOutOfRangeException("index");
            }

            this.CompressedSize = compressedSize;
            this.Hash = hash;
            this.Index = index;
        }

        /// <summary>
        /// Gets the compressed size of the block.
        /// </summary>
        public ulong CompressedSize { get; }

        /// <summary>
        /// Gets the hash of the block.
        /// </summary>
        public string Hash { get; }

        /// <summary>
        /// Gets the zero-based index of the block.
        /// </summary>
        public int Index { get; }

        /// <summary>
        /// Creates a new instance of the Block class from a given IAppxBlockMapBlock and index.
        /// The Block's hash will be Base64 encoded.
        /// </summary>
        /// <param name="block">Blockmap block</param>
        /// <param name="index">Block index</param>
        /// <returns>New Block instance</returns>
        public static Block CreateFromAppxBlockMapBlock(IAppxBlockMapBlock block, int index)
        {
            ulong compressedSize = block.GetCompressedSize();

            // get hash size & pointer to hash buffer
            uint hashSize;
            IntPtr hashPtr = block.GetHash(out hashSize);

            // copy hash into byte array
            byte[] hash = new byte[hashSize];
            Marshal.Copy(hashPtr, hash, 0, (int)hashSize);

            // free hash buffer
            Marshal.FreeCoTaskMem(hashPtr);

            // convert hash to base64 string
            string hashB64 = Convert.ToBase64String(hash);

            return new Block(compressedSize, hashB64, index);
        }

        /// <summary>
        /// Compares this object with other for equality.
        /// </summary>
        /// <param name="obj">The object to compare against.</param>
        /// <returns>True if this objects is equal to the argument, false otherwise.</returns>
        public override bool Equals(object obj)
        {
            var other = obj as Block;
            if (other == null)
            {
                return false;
            }

            return this.CompressedSize == other.CompressedSize
                && this.Index == other.Index
                && this.Hash.Equals(other.Hash);
        }

        /// <summary>
        /// Gets a hash code for this object.
        /// </summary>
        /// <returns>A hash code for this object.</returns>
        public override int GetHashCode()
        {
            return this.Hash.GetHashCode();
        }
    }
}
