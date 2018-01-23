
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>

/* ================================================================== */
// Global variables 
/* ================================================================== */

UINT64 volatile allCount = 0;        //number of dynamically executed instructions
UINT64 bblCount = 0;        //number of dynamically executed basic blocks
UINT64 threadCount = 0;     //total number of threads, including main thread
UINT64 nopCount = 0;
UINT64 DCallCount = 0;
UINT64 ICallCount = 0;
UINT64 retCount = 0;
UINT64 unBraCount = 0;
UINT64 braCount = 0;
UINT64 loOpCount = 0;
UINT64 shiCount = 0;
UINT64 flOpCount = 0;
UINT64 vecCount = 0;
UINT64 coMovCount = 0;
UINT64 MMXSSECount = 0;
UINT64 sysCount = 0;
UINT64 flPtCount = 0;
UINT64 otherCount = 0;
UINT64 ldCount = 0;
UINT64 stCount = 0;

UINT64 G = 32;

UINT64 benchLength = 1000000000;
UINT64 last = 0;

map < UINT64,bool >  instrMap;

map < UINT64,bool >  dataMap;

std::ostream * out = &cerr;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for MyPinTool output");

KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool",
    "count", "1", "count instructions, basic blocks and threads in the application");

KNOB<UINT64>   fastForward(KNOB_MODE_WRITEONCE,  "pintool",
    "f", "0", "specify the fast forward value");


/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl <<
            "instructions, basic blocks and threads in the application." << endl << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

/*!
 * Increase counter of the executed basic blocks and instructions.
 * This function is called for every basic block when it is about to be executed.
 * @param[in]   numInstInBbl    number of instructions in the basic block
 * @note use atomic operations for multi-threaded applications
 */

// VOID CountBbl(UINT64 numInstInBbl)
// {
//     bblCount++;
//     allCount += numInstInBbl;
// }

VOID allInstruction()
{
    allCount++;
}


VOID nopInstruction()
{
    nopCount++;
}

VOID DCallInstruction()
{
    DCallCount++;
}

VOID ICallInstruction()
{
    ICallCount++;
}

VOID retInstruction()
{
    retCount++;
}

VOID unBraInstruction()
{
    unBraCount++;
}

VOID braInstruction()
{
    braCount++;
}

VOID loOpInstruction()
{
    loOpCount++;
}

VOID shiInstruction()
{
    shiCount++;
}

VOID flOpInstruction()
{
    flOpCount++;
}

VOID vecInstruction()
{
    vecCount++;
}

VOID coMovInstruction()
{
    coMovCount++;
}

VOID MMXSSEInstruction()
{
    MMXSSECount++;
}

VOID sysInstruction()
{
    sysCount++;
}

VOID flPtInstruction()
{
    flPtCount++;
}

VOID otherInstruction()
{
    otherCount++;
}

VOID ldInstruction(UINT64 refSize)
{
    ldCount += refSize;
}

VOID stInstruction(UINT64 refSize)
{
    stCount += refSize;
}


VOID insMemory(VOID *p, UINT64 insSize)
{
	// (*instrVec).push_back(make_pair((reinterpret_cast<UINT64> (p))/G, (reinterpret_cast<UINT64> (p) + insSize)/G) );
	UINT64 start = reinterpret_cast<UINT64> (p)/G;
	UINT64 end = (reinterpret_cast<UINT64> (p) + insSize)/G;

	for(UINT64 i = start;i<=end;i++)
	{
		instrMap[i] = true;
	}

}

VOID dataMemory(VOID *p, UINT64 dataSize)
{
	UINT64 start = reinterpret_cast<UINT64> (p)/G;
	UINT64 end = (reinterpret_cast<UINT64> (p) + dataSize)/G;

	for(UINT64 i = start;i<=end;i++)
	{
		dataMap[i] = true;
	}

}





ADDRINT Terminate(void)
{
    return (allCount >= (fastForward*benchLength + benchLength));
}

ADDRINT FastForward(void) {
    return (allCount >= fastForward*benchLength && allCount < fastForward*benchLength + benchLength);
}

VOID MyExitRoutine() {

    // printf("allCount = %u\n", allCount);
    // printf("nopCount = %u\n", nopCount);
    // printf("DCallCount = %u\n", DCallCount);
    // printf("ICallCount = %u\n", ICallCount);
    // printf("retCount = %u\n", retCount);
    // printf("unBraCount = %u\n", unBraCount);
    // printf("braCount = %u\n", braCount);
    // printf("loOpCount = %u\n", loOpCount);
    // printf("shiCount = %u\n", shiCount);
    // printf("flOpCount = %u\n", flOpCount);
    // printf("vecCount = %u\n", vecCount);
    // printf("coMovCount = %u\n", coMovCount);
    // printf("MMXSSECount = %u\n", MMXSSECount);
    // printf("sysCount = %u\n", sysCount);
    // printf("flPtCount = %u\n", flPtCount);
    // printf("otherCount = %u\n", otherCount);
    // printf("ldCount = %u\n", ldCount);
    // printf("stCount = %u\n", stCount);

    *out << "allCount = " << allCount << endl;
    *out << "nopCount = " << nopCount << endl;
    *out << "DCallCount = " << DCallCount << endl;
    *out << "ICallCount = " << ICallCount << endl;
    *out << "retCount = " << retCount << endl;
    *out << "unBraCount = " << unBraCount << endl;
    *out << "braCount = " << braCount << endl;
    *out << "loOpCount = " << loOpCount << endl;
    *out << "shiCount = " << shiCount << endl;
    *out << "flOpCount = " << flOpCount << endl;
    *out << "vecCount = " << vecCount << endl;
    *out << "coMovCount = " << coMovCount << endl;
    *out << "MMXSSECount = " << MMXSSECount << endl;
    *out << "sysCount = " << sysCount << endl;
    *out << "flPtCount = " << flPtCount << endl;
    *out << "otherCount = " << otherCount << endl;
    *out << "ldCount = " << ldCount << endl;
    *out << "stCount = " << stCount << endl;
    *out << "CPI = " << (nopCount + DCallCount + ICallCount + retCount + unBraCount + braCount + loOpCount + shiCount + flOpCount + vecCount + coMovCount + MMXSSECount + sysCount + flPtCount + otherCount + ldCount * 50.0 + stCount * 50.0)/(UINT64)benchLength << endl;
    
    

    // sort((*instrVec).begin(), (*instrVec).end());
    // UINT64 total=0;
    // UINT64 start,end;

    // for (UINT64 i = 0; i < (*instrVec).size(); ++i)
    // {
    // 	start = (*instrVec)[i].first;
    // 	end = (*instrVec)[i].second;
    // 	while (i+1<(*instrVec).size() && (*instrVec)[i+1].first <= end)
    // 	{
    // 		end = max((*instrVec)[i+1].second,end);
    // 		i++;
    // 	}
    // 	total += end-start+1;

    // }

    *out << "Instruction Memory Map = " << G*instrMap.size() <<endl;
    *out << "Data Memory Map = " << G*dataMap.size() <<endl;



    exit(0);
}

// VOID MyPredicatedAnalysis() {
// // # analysis code
// }


/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 * Insert call to the CountBbl() analysis routine before every basic block 
 * of the trace.
 * This function is called every time a new trace is encountered.
 * @param[in]   trace    trace to be instrumented
 * @param[in]   v        value specified by the tool in the TRACE_AddInstrumentFunction
 *                       function call
 */

// VOID Trace(TRACE trace, VOID *v)
// {
//     // Visit every basic block in the trace
//     for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
//     {
//         // Insert a call to CountBbl() before every basic bloc, passing the number of instructions
//         BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CountBbl, IARG_UINT64, BBL_NumIns(bbl), IARG_END);
//     }
// }

VOID Instruction(INS ins, void *v)
{
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) Terminate, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)MyExitRoutine, IARG_END);
    
    //Benchmarking
    if ((allCount)/benchLength > last)
    {
        cerr << last + 1 << " / 208" <<endl;
        cerr << allCount << endl;
        last = (allCount)/benchLength;
    }

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)allInstruction, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
    
    if (INS_Category(ins) == XED_CATEGORY_NOP)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)nopInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_CALL)
    {
        if (INS_IsDirectCall(ins))
        {
            // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)DCallInstruction, IARG_END);
        }
        else {
            // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)ICallInstruction, IARG_END);
        }
    }
    else if (INS_Category(ins) == XED_CATEGORY_RET)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)retInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_UNCOND_BR)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)unBraInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_COND_BR)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)braInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_LOGICAL)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)loOpInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_SHIFT || INS_Category(ins) == XED_CATEGORY_ROTATE)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)shiInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_FLAGOP)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)flOpInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_AVX2 || INS_Category(ins) == XED_CATEGORY_AVX || INS_Category(ins) == XED_CATEGORY_AVX2GATHER)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)vecInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_CMOV)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)coMovInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_SSE || INS_Category(ins) == XED_CATEGORY_MMX)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)MMXSSEInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_SYSCALL)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)sysInstruction, IARG_END);
    }
    else if (INS_Category(ins) == XED_CATEGORY_X87_ALU)
    {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)flPtInstruction, IARG_END);
    }
    else {
        // INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)otherInstruction, IARG_END);
    }

    UINT64 memOperands = INS_MemoryOperandCount(ins);
    for (UINT32 memOp = 0; memOp < memOperands; ++memOp)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            UINT64 memSize = INS_MemoryOperandSize(ins, memOp);
            UINT64 refSize;
            refSize = (memSize+(UINT64)3)/(UINT64)4;
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)ldInstruction, IARG_UINT64, refSize, IARG_END);


    		INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
   			INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)dataMemory, IARG_MEMORYOP_EA, memOp, IARG_UINT64, memSize, IARG_END);


        }
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            UINT64 memSize = INS_MemoryOperandSize(ins, memOp);
            UINT64 refSize;
            refSize = (memSize+(UINT64)3)/(UINT64)4;
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)stInstruction, IARG_UINT64, refSize, IARG_END);


            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
   			INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)dataMemory, IARG_MEMORYOP_EA, memOp, IARG_UINT64, memSize, IARG_END);
        }
    }

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
    UINT64 insSize = INS_Size(ins);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)insMemory, IARG_INST_PTR, IARG_UINT64, insSize, IARG_END);

}

/*!
 * Increase counter of threads in the application.
 * This function is called for every thread created by the application when it is
 * about to start running (including the root thread).
 * @param[in]   threadIndex     ID assigned by PIN to the new thread
 * @param[in]   ctxt            initial register state for the new thread
 * @param[in]   flags           thread creation flags (OS specific)
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddThreadStartFunction function call
 */
// VOID ThreadStart(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
// {
//     threadCount++;
// }

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v)
{
    *out <<  "===============================================" << endl;
    *out <<  "MyPinTool analysis results: " << endl;
    *out <<  "Number of instructions: " << allCount  << endl;
    *out << "allCount = " << allCount << endl;
    *out << "nopCount = " << nopCount << endl;
    *out << "DCallCount = " << DCallCount << endl;
    *out << "ICallCount = " << ICallCount << endl;
    *out << "retCount = " << retCount << endl;
    *out << "unBraCount = " << unBraCount << endl;
    *out << "braCount = " << braCount << endl;
    *out << "loOpCount = " << loOpCount << endl;
    *out << "shiCount = " << shiCount << endl;
    *out << "flOpCount = " << flOpCount << endl;
    *out << "vecCount = " << vecCount << endl;
    *out << "coMovCount = " << coMovCount << endl;
    *out << "MMXSSECount = " << MMXSSECount << endl;
    *out << "sysCount = " << sysCount << endl;
    *out << "flPtCount = " << flPtCount << endl;
    *out << "otherCount = " << otherCount << endl;
    *out << "ldCount = " << ldCount << endl;
    *out << "stCount = " << stCount << endl;
    *out << "CPI = " << (nopCount + DCallCount + ICallCount + retCount + unBraCount + braCount + loOpCount + shiCount + flOpCount + vecCount + coMovCount + MMXSSECount + sysCount + flPtCount + otherCount + ldCount * 50.0 + stCount * 50.0)/(UINT64)benchLength << endl;
    // *out <<  "Number of basic blocks: " << bblCount  << endl;
    // *out <<  "Number of threads: " << threadCount  << endl;




    // sort((*instrVec).begin(), (*instrVec).end());
    // UINT64 total=0;
    // UINT64 start,end;

    // for (UINT64 i = 0; i < (*instrVec).size(); ++i)
    // {
    // 	start = (*instrVec)[i].first;
    // 	end = (*instrVec)[i].second;
    // 	while (i+1<(*instrVec).size() && (*instrVec)[i+1].first <= end)
    // 	{
    // 		end = max((*instrVec)[i+1].second,end);
    // 		i++;
    // 	}
    // 	total += end-start+1;

    // }

    // *out << "Instruction Memory Map = " << total*G <<endl;



    *out << "Instruction Memory Map = " << G*(instrMap.size()) <<endl;
    *out << "Data Memory Map = " << G*dataMap.size() <<endl;
    *out <<  "===============================================" << endl;
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid 

    // (instrVec) = new vector <pair<UINT64,UINT64> > [benchLength];
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }
    
    string fileName = KnobOutputFile.Value();

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

    if (KnobCount)
    {
        // Register function to be called to instrument traces
        INS_AddInstrumentFunction(Instruction, 0);

        // Register function to be called for every thread before it starts running
        // PIN_AddThreadStartFunction(ThreadStart, 0);

        // Register function to be called when the application exits
        PIN_AddFiniFunction(Fini, 0);
    }
    
    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by MyPinTool" << endl;
    if (!KnobOutputFile.Value().empty()) 
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr <<  "===============================================" << endl;


    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
