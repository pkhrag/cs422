
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

UINT64 maxBytesTouched = 0;
UINT64 totalBytesTouched = 0;
UINT64 countMemInst = 0;
INT32 maxImmediate = INT32_MIN;
INT32 minImmediate = INT32_MAX;
ADDRDELTA maxDis = 0;
ADDRDELTA minDis = UINT32_MAX;


UINT64 G = 32;

UINT64 benchLength = 1000000000;
UINT64 last = 0;

map < UINT64,bool >  instrMap;

map < UINT64,bool >  dataMap;

UINT64 D1[50] = {0};
UINT64 D2[50] = {0};
UINT64 D3[50] = {0};
UINT64 D4[50] = {0};
UINT64 D5[50] = {0};
UINT64 D6[50] = {0};
UINT64 D7[50] = {0};
UINT64 D8[50] = {0};
UINT64 D9[50] = {0};
UINT64 D10[50] = {0};

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


VOID insMemory(VOID *p, UINT64 insSize, UINT64 insOp, UINT64 insROp, UINT64 insWOp)
{

	UINT64 start = reinterpret_cast<UINT64> (p)/G;
	UINT64 end = (reinterpret_cast<UINT64> (p) + insSize)/G;

	for(UINT64 i = start;i<=end;i++)
	{
		instrMap[i] = true;
	}

	D1[insSize] = D1[insSize] + 1;
	D2[insOp] = D2[insOp] + 1;
	D3[insROp] = D3[insROp] + 1;
	D4[insWOp] = D4[insWOp] + 1;

}

VOID dataMemory(VOID *p, UINT64 dataSize, ADDRDELTA dis)
{
	UINT64 start = (reinterpret_cast<UINT64> (p))/G;
	UINT64 end = ((reinterpret_cast<UINT64> (p)) + dataSize)/G;

	for(UINT64 i = start;i<=end;i++)
	{
		dataMap[i] = true;
	}

	maxDis = max(dis, maxDis);
	minDis = min(dis, minDis);
}

VOID dataDistribution(UINT64 memOp, UINT64 memROp, UINT64 memWOp, UINT64 totalSize, UINT64 yesMem)
{
	D5[memROp + memWOp]++;
    if (memROp + memWOp) {
        D6[memROp]++;
        D7[memWOp]++;
    }

	if(totalSize > maxBytesTouched)
		maxBytesTouched = totalSize;

	totalBytesTouched += totalSize;

	countMemInst += yesMem;

}

VOID immDistribution(INT32 ret)
{
	maxImmediate = max(ret, maxImmediate);
	minImmediate = min(ret, minImmediate);
}



ADDRINT Terminate(void)
{
    return (allCount >= (fastForward*benchLength + benchLength));
}

ADDRINT FastForward(void) {
    return (allCount >= fastForward*benchLength && allCount < fastForward*benchLength + benchLength);
}

VOID MyExitRoutine() {

    *out <<  "===============================================" << endl;
    *out <<  "MyPinTool analysis results: " << endl;
    *out << "Total Instructions: " << allCount << endl;
    *out << "NOP Instructions: " << nopCount << endl;
    *out << "Directed Call Instructions: " << DCallCount << endl;
    *out << "Indirected Call Instructions: " << ICallCount << endl;
    *out << "Return Instructions: " << retCount << endl;
    *out << "Unconditional Branch Instructions: " << unBraCount << endl;
    *out << "Conditional Branch Instructions: " << braCount << endl;
    *out << "Logical Operation Instructions: " << loOpCount << endl;
    *out << "Shift Instructions: " << shiCount << endl;
    *out << "Flag Operation Instructions: " << flOpCount << endl;
    *out << "Vector Instructions: " << vecCount << endl;
    *out << "Conditional Moves Instructions: " << coMovCount << endl;
    *out << "MME, SSE Instructions: " << MMXSSECount << endl;
    *out << "System Instructions: " << sysCount << endl;
    *out << "Floating Point Instructions: " << flPtCount << endl;
    *out << "Other Instructions: " << otherCount << endl;
    *out << "Load Instructions: " << ldCount << endl;
    *out << "Store Instructions: " << stCount << endl;
    *out << "CPI: " << (nopCount + DCallCount + ICallCount + retCount + unBraCount + braCount + loOpCount + shiCount + flOpCount + vecCount + coMovCount + MMXSSECount + sysCount + flPtCount + otherCount + ldCount * 50.0 + stCount * 50.0)/(UINT64)benchLength << endl;
    *out << "Instruction Memory Map: " << G*instrMap.size() <<endl;
    *out << "Data Memory Map: " << G*dataMap.size() <<endl;

    *out << "Instruction Length Distribution: " << endl;
    UINT32 it;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D1[it]<<endl; 
    }

    *out << "Distribution of number of operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D2[it]<<endl; 
    }

    *out << "Distribution of number of register read operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D3[it]<<endl; 
    }

    *out << "Distribution of number of register write operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D4[it]<<endl; 
    }

    *out << "Distribution of number of memory operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D5[it]<<endl; 
    }

    *out << "Distribution of number of read memory operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D6[it]<<endl; 
    }

    *out << "Distribution of number of wirte memory operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D7[it]<<endl; 
    }
    *out << "Maximum Memory Bytes Touched: " << maxBytesTouched << endl;
    *out << "Average Bytes Touched: " << (totalBytesTouched * 1.0) / countMemInst << endl;
    *out << "Maximum Immediate Field: " << maxImmediate << endl;
    *out << "Minimum Immediate Field: " << minImmediate << endl;
    *out << "Maximum Displacement Field: " << maxDis << endl;
    *out << "Minimum Displacement Field: " << minDis << endl;
    *out <<  "===============================================" << endl;

    exit(0);
}

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
    UINT64 memROp = 0;
    UINT64 memWOp = 0;
    UINT64 yesMem = 0;

    UINT64 totalSize = 0;
    for (UINT32 memOp = 0; memOp < memOperands; ++memOp)
    {
    	yesMem = 1;
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            UINT64 memSize = INS_MemoryOperandSize(ins, memOp);
            UINT64 refSize;
            refSize = (memSize+3)/4;
            memROp++;
        	ADDRDELTA dis = INS_OperandMemoryDisplacement(ins, memOp);

            totalSize += memSize;
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)ldInstruction, IARG_UINT64, refSize, IARG_END);

    		INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
   			INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)dataMemory, IARG_MEMORYOP_EA, memOp, IARG_UINT64, memSize , IARG_ADDRINT, dis, IARG_END);
        }

        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            UINT64 memSize = INS_MemoryOperandSize(ins, memOp);
            UINT64 refSize;
            memWOp++;
            refSize = (memSize+3)/4;
        	ADDRDELTA dis = INS_OperandMemoryDisplacement(ins, memOp);

            totalSize += memSize;
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)stInstruction, IARG_UINT64, refSize, IARG_END);

            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
   			INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)dataMemory, IARG_MEMORYOP_EA, memOp, IARG_UINT64, memSize, IARG_ADDRINT, dis, IARG_END);
        }
    }


    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
	INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)dataDistribution, IARG_UINT64, memOperands, IARG_UINT64, memROp, IARG_UINT64, memWOp,IARG_UINT64, totalSize, IARG_UINT64, yesMem, IARG_END);


    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
    UINT64 insSize = INS_Size(ins);
    UINT64 insOp = INS_OperandCount(ins);
    UINT64 insROp = INS_MaxNumRRegs(ins);
    UINT64 insWOp = INS_MaxNumWRegs(ins);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)insMemory, IARG_INST_PTR, IARG_UINT64, insSize, IARG_UINT64, insOp, IARG_UINT64, insROp, IARG_UINT64, insWOp, IARG_END);

    UINT64 operandsNo = INS_OperandCount(ins);
    for (UINT32 Op = 0; Op < operandsNo; ++Op)
    {
    	
        BOOL isImmediate = INS_OperandIsImmediate(ins,Op);
        if(isImmediate)
        {
        	INT32 ret = INS_OperandImmediate(ins,Op);
	    	INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
    		INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)immDistribution, IARG_ADDRINT, ret, IARG_END);
        }
    }



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
    *out << "Total Instructions: " << allCount << endl;
    *out << "NOP Instructions: " << nopCount << endl;
    *out << "Directed Call Instructions: " << DCallCount << endl;
    *out << "Indirected Call Instructions: " << ICallCount << endl;
    *out << "Return Instructions: " << retCount << endl;
    *out << "Unconditional Branch Instructions: " << unBraCount << endl;
    *out << "Conditional Branch Instructions: " << braCount << endl;
    *out << "Logical Operation Instructions: " << loOpCount << endl;
    *out << "Shift Instructions: " << shiCount << endl;
    *out << "Flag Operation Instructions: " << flOpCount << endl;
    *out << "Vector Instructions: " << vecCount << endl;
    *out << "Conditional Moves Instructions: " << coMovCount << endl;
    *out << "MME, SSE Instructions: " << MMXSSECount << endl;
    *out << "System Instructions: " << sysCount << endl;
    *out << "Floating Point Instructions: " << flPtCount << endl;
    *out << "Other Instructions: " << otherCount << endl;
    *out << "Load Instructions: " << ldCount << endl;
    *out << "Store Instructions: " << stCount << endl;
    *out << "CPI: " << (nopCount + DCallCount + ICallCount + retCount + unBraCount + braCount + loOpCount + shiCount + flOpCount + vecCount + coMovCount + MMXSSECount + sysCount + flPtCount + otherCount + ldCount * 50.0 + stCount * 50.0)/(UINT64)benchLength << endl;
    *out << "Instruction Memory Map: " << G*instrMap.size() <<endl;
    *out << "Data Memory Map: " << G*dataMap.size() <<endl;

    *out << "Instruction Length Distribution: " << endl;
    UINT32 it;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D1[it]<<endl; 
    }

    *out << "Distribution of number of operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D2[it]<<endl; 
    }

    *out << "Distribution of number of register read operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D3[it]<<endl; 
    }

    *out << "Distribution of number of register write operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D4[it]<<endl; 
    }

    *out << "Distribution of number of memory operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D5[it]<<endl; 
    }

    *out << "Distribution of number of read memory operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D6[it]<<endl; 
    }

    *out << "Distribution of number of wirte memory operands: " << endl;
    for(it=0;it!=50;it++)
    {
    	*out << it << " = " << D7[it]<<endl; 
    }
    *out << "Maximum Memory Bytes Touched: " << maxBytesTouched << endl;
    *out << "Average Bytes Touched: " << (totalBytesTouched * 1.0) / countMemInst << endl;
    *out << "Maximum Immediate Field: " << maxImmediate << endl;
    *out << "Minimum Immediate Field: " << minImmediate << endl;
    *out << "Maximum Displacement Field: " << maxDis << endl;
    *out << "Minimum Displacement Field: " << minDis << endl;
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
