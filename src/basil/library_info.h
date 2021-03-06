// ==========================================================================
//                                 BASIL
// ==========================================================================
// Copyright (c) 2006-2014, Knut Reinert, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Manuel Holtgrewe <manuel.holtgrewe@fu-berlin.de>
// ==========================================================================

#ifndef SANDBOX_HERBARIUM_APPS_BASIL_LIBRARY_INFO_H_
#define SANDBOX_HERBARIUM_APPS_BASIL_LIBRARY_INFO_H_

#include <seqan/bam_io.h>

// ============================================================================
// Forwards
// ============================================================================

// ============================================================================
// Tags, Classes, Enums
// ============================================================================

// ----------------------------------------------------------------------------
// Class BamLibraryInfo
// ----------------------------------------------------------------------------

// Represents the orientation and insert size of paired reads.

struct BamLibraryInfo
{
    // This is the notation introduced by Illumina.
    //
    // F+: >R1 R2>
    // F-: >R2 R1>
    // R+: >R1 R2<
    // R-: <R2 R1>
    enum Orientation
    {
        F_PLUS  = 0,
        F_MINUS = 1,
        R_PLUS  = 2,
        R_MINUS = 3
    };

    unsigned median;
    double   stdDev;
    unsigned maxNormalISize;
    Orientation defaultOrient;

    BamLibraryInfo() : median(0), stdDev(0), maxNormalISize(0), defaultOrient(F_PLUS)
    {}
};

// ----------------------------------------------------------------------------
// Class BamLibraryEstimator
// ----------------------------------------------------------------------------

// Allows to detect LibraryInfo information from BAM file.

class BamLibraryEstimator
{
public:
    // Number of records to evaluate.
    int numRecords;

    BamLibraryEstimator(int numRecords = 100*1000) : numRecords(numRecords)
    {}

    inline BamLibraryInfo::Orientation
    getStrandIndependentOrientation(seqan::BamAlignmentRecord const & rec);

    int run(BamLibraryInfo & result, char const * path);
};

// ============================================================================
// Metafunctions
// ============================================================================

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function getOrientationStr()
// ----------------------------------------------------------------------------

inline char const * getOrientationStr(BamLibraryInfo::Orientation o)
{
    switch (o)
    {
        case BamLibraryInfo::F_PLUS:
            return "F+ >R1 R2>";
        case BamLibraryInfo::F_MINUS:
            return "F- >R2 R1>";
        case BamLibraryInfo::R_PLUS:
            return "R+ >R1 R2<";
        case BamLibraryInfo::R_MINUS:
            return "R- <R1 R2>";
        default:
            return "INVALID";
    }
}

// ----------------------------------------------------------------------------
// Function endsWith()
// ----------------------------------------------------------------------------

inline bool endsWith(seqan::CharString const & str, seqan::CharString const & x)
{
    typedef seqan::Size<seqan::CharString>::Type TSize;

    TSize len = std::min(length(str), length(x));
    TSize pos = length(str) - len;
    return suffix(str, pos) == x;
}

// ----------------------------------------------------------------------------
// Function BamLibraryEstimator::getStrandIndependentOrientation()
// ----------------------------------------------------------------------------

inline BamLibraryInfo::Orientation
BamLibraryEstimator::getStrandIndependentOrientation(seqan::BamAlignmentRecord const & rec)
{
    if (!hasFlagRC(rec))
    {
        if (!(hasFlagNextRC(rec)))
            return (rec.beginPos < rec.pNext) ? BamLibraryInfo::F_PLUS : BamLibraryInfo::F_MINUS;
        else
            return (rec.beginPos < rec.pNext) ? BamLibraryInfo::R_PLUS : BamLibraryInfo::R_MINUS;
    } else {
        if (!hasFlagNextRC(rec))
            return (rec.beginPos > rec.pNext) ? BamLibraryInfo::R_PLUS : BamLibraryInfo::R_MINUS;
        else
            return (rec.beginPos > rec.pNext) ? BamLibraryInfo::F_PLUS : BamLibraryInfo::F_MINUS;
    }
}

// ----------------------------------------------------------------------------
// Function BamLibraryEstimator::run()
// ----------------------------------------------------------------------------

inline int
BamLibraryEstimator::run(BamLibraryInfo & result, char const * path)
{
    // Vector of all insert sizes.
    typedef std::vector<unsigned> TVecISize;
    TVecISize vecISize;

    // Open SAM/BAM stream and read header.
    seqan::BamFileIn bamFileIn;
    if (!open(bamFileIn, path))
    {
        std::cerr << "ERROR: Could not open SAM/BAM file " << path << "\n";
        return 1;
    }
    seqan::BamHeader header;
    readHeader(header, bamFileIn);

    // Orientations.
    unsigned orientationCounters[4] = {0, 0, 0, 0};

    // Running mean.
    __uint64 runningMean = 0;
    unsigned pairCount = 0;

    // Count.
    seqan::BamAlignmentRecord record;
    for (int i = 0; !atEnd(bamFileIn) && (numRecords == 0 || i < numRecords); ++i)
    {
        try
        {
            readRecord(record, bamFileIn);
        }
        catch (seqan::ParseError const & e)
        {
            std::cerr << "Error reading SAM/BAM file.\n";
            return 1;
        }

        // Skip all records except for first mate of properly mapped pairs.
        if (!hasFlagMultiple(record) || hasFlagUnmapped(record) || hasFlagNextUnmapped(record) ||
            hasFlagSecondary(record) || hasFlagFirst(record))
            continue;

        vecISize.push_back(abs(record.tLen));
        runningMean += abs(record.tLen);
        pairCount += 1;

        orientationCounters[getStrandIndependentOrientation(record)] += 1;
    }

    // Get default orientation.
    unsigned orientMax = 0;
    for (unsigned i = 0; i < 4; ++i)
        if (orientationCounters[i] > orientationCounters[orientMax])
            orientMax = i;
    BamLibraryInfo::Orientation orientations[4] = { BamLibraryInfo::F_PLUS, BamLibraryInfo::F_MINUS,
                                                    BamLibraryInfo::R_PLUS, BamLibraryInfo::R_MINUS };
    result.defaultOrient = orientations[orientMax];

    // Trim off the chimera peak in mate-pair libraries.
    if (result.defaultOrient == BamLibraryInfo::R_MINUS && (runningMean / pairCount) >= 1000u)
    {
        typedef TVecISize::const_iterator TVecISizeIter;
        TVecISize vecISizeTmp;
        for(TVecISizeIter it = vecISize.begin(); it < vecISize.end(); ++it)
            if (*it > 1000)
                vecISizeTmp.push_back(*it);
        std::swap(vecISize, vecISizeTmp);
    }

    // Check that this is a proper paired-end library
    if (vecISize.empty())
        return 0;

    // Get library stats.
    //
    // Start with median.
    typedef TVecISize::iterator TVecISizeIter;
    TVecISizeIter begin = vecISize.begin();
    TVecISizeIter end = vecISize.end();
    std::nth_element(begin, begin + (end - begin) / 2, end);
    begin = vecISize.begin();
    end = vecISize.end();
    result.median = *(begin + (end - begin) / 2);
    // Standard deviation is next.
    //
    // SD calculation cutoffs are 7 SDs to the left and right assuming 10% library deviation.
    result.stdDev = 0;
    double cutoffMax = result.median + 7 * 0.1 * result.median;
    double cutoffMin = result.median - 7 * 0.1 * result.median;
    if ((cutoffMin < 0) || (cutoffMax < cutoffMin))
        cutoffMin = 0; 
    unsigned int count = 0;
    for(;begin < end; ++begin)
    {
        if ((*begin >= cutoffMin) && (*begin <= cutoffMax))
        {
            result.stdDev += (*begin - result.median) * (*begin - result.median);
            ++count;
        }
    }
    result.stdDev = sqrt(result.stdDev / count);
    result.maxNormalISize = static_cast<unsigned>(result.median + 3 * result.stdDev);

    return 0;
}

#endif  // #ifndef SANDBOX_HERBARIUM_APPS_BASIL_LIBRARY_INFO_H_
