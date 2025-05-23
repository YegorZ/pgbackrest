/***********************************************************************************************************************************
Harness for Protocol Testing
***********************************************************************************************************************************/
#include "build.auto.h"

#include <stdlib.h>
#include <unistd.h>

#include "common/fork.h"
#include "common/io/fdRead.h"
#include "common/io/fdWrite.h"

#include "common/harnessConfig.h"
#include "common/harnessDebug.h"
#include "common/harnessLog.h"
#include "common/harnessProtocol.h"

/***********************************************************************************************************************************
Include shimmed C modules
***********************************************************************************************************************************/
{[SHIM_MODULE]}

/***********************************************************************************************************************************
Shim install state
***********************************************************************************************************************************/
static struct
{
    // Local process shim
    bool localShim;
    const List *localHandlerList;

    // Remote process shim
    bool remoteShim;
    const List *remoteHandlerList;
} hrnProtocolStatic;

/***********************************************************************************************************************************
Cleanup all clients inherited from the parent process so they cannot be accidentally used to send messages to servers that do not
belong to this process. We need to do this carefully so that exit commands are not sent and processes are not terminated, so clear
the mem context callback on each object before freeing it.
***********************************************************************************************************************************/
static void
hrnProtocolClientCleanup(void)
{
    FUNCTION_LOG_VOID(logLevelDebug);

    if (protocolHelper.clientList != NULL)
    {
        // Cleanup remotes
        while (lstSize(protocolHelper.clientList) > 0)
        {
            ProtocolHelperClient *const clientHelper = lstGet(protocolHelper.clientList, 0);

            memContextCallbackClear(objMemContext(clientHelper->client));
            protocolClientFree(clientHelper->client);
            lstRemoveIdx(protocolHelper.clientList, 0);

            if (clientHelper->exec != NULL)
            {
                memContextCallbackClear(objMemContext(clientHelper->exec));
                execFree(clientHelper->exec);
            }
        }
    }

    FUNCTION_LOG_RETURN_VOID();
}

/***********************************************************************************************************************************
Shim protocolLocalExec() to provide coverage as detailed in the hrnProtocolLocalShimInstall() documentation.
***********************************************************************************************************************************/
static void
protocolLocalExec(
    ProtocolHelperClient *const helper, ProtocolStorageType const protocolStorageType, const unsigned int hostIdx,
    const unsigned int processId)
{
    // Call the shim when installed
    if (hrnProtocolStatic.localShim)
    {
        FUNCTION_LOG_BEGIN(logLevelDebug);
            FUNCTION_LOG_PARAM_P(VOID, helper);
            FUNCTION_LOG_PARAM(STRING_ID, protocolStorageType);
            FUNCTION_LOG_PARAM(UINT, hostIdx);
            FUNCTION_LOG_PARAM(UINT, processId);
        FUNCTION_LOG_END();

        // Create pipes to communicate with the subprocess. The names of the pipes are from the perspective of the parent process
        // since the child process will use them only briefly before exec'ing.
        int pipeRead[2];
        int pipeWrite[2];

        THROW_ON_SYS_ERROR(pipe(pipeRead) == -1, KernelError, "unable to create read pipe");
        THROW_ON_SYS_ERROR(pipe(pipeWrite) == -1, KernelError, "unable to create write pipe");

        // Exec command in the child process
        if (forkSafe() == 0)
        {
            // Cleanup inherited clients
            hrnProtocolClientCleanup();

            // Load configuration
            StringList *const paramList = protocolLocalParam(protocolStorageType, hostIdx, processId);
            hrnCfgLoadP(CFG_COMMAND_TOTAL, paramList, .noStd = true);

            // Change log process id to aid in debugging
            hrnLogProcessIdSet(processId);

            // Run server with provided handlers
            String *name = strNewFmt(PROTOCOL_SERVICE_LOCAL "-shim-%u", processId);
            ProtocolServer *server = protocolServerNew(
                name, PROTOCOL_SERVICE_LOCAL_STR, ioFdReadNewOpen(name, pipeWrite[0], 5000),
                ioFdWriteNewOpen(name, pipeRead[1], 5000));
            protocolServerProcess(server, cfgCommandJobRetry(), hrnProtocolStatic.localHandlerList);

            // Exit when done
            exit(0);
        }

        // Close the unused file descriptors
        close(pipeRead[1]);
        close(pipeWrite[0]);

        // Create protocol object
        helper->client = protocolClientNew(
            strNewFmt(PROTOCOL_SERVICE_LOCAL "-%u shim protocol", processId), PROTOCOL_SERVICE_LOCAL_STR,
            ioFdReadNewOpen(strNewFmt(PROTOCOL_SERVICE_LOCAL "-%u shim protocol read", processId), pipeRead[0], 5000),
            ioFdWriteNewOpen(strNewFmt(PROTOCOL_SERVICE_LOCAL "-%u shim protocol write", processId), pipeWrite[1], 5000));

        // Send one noop to catch any errors that might happen after the greeting
        protocolClientNoOp(helper->client);

        FUNCTION_LOG_RETURN_VOID();
    }
    // Else call the base function
    else
        protocolLocalExec_SHIMMED(helper, protocolStorageType, hostIdx, processId);
}

/**********************************************************************************************************************************/
void
hrnProtocolLocalShimInstall(const List *const handlerList)
{
    FUNCTION_HARNESS_BEGIN();
        FUNCTION_HARNESS_PARAM(LIST, handlerList);
    FUNCTION_HARNESS_END();

    hrnProtocolStatic.localShim = true;
    hrnProtocolStatic.localHandlerList = handlerList;

    FUNCTION_HARNESS_RETURN_VOID();
}

/**********************************************************************************************************************************/
void
hrnProtocolLocalShimUninstall(void)
{
    FUNCTION_HARNESS_VOID();

    hrnProtocolStatic.localShim = false;

    FUNCTION_HARNESS_RETURN_VOID();
}

/***********************************************************************************************************************************
Shim protocolRemoteExec() to provide coverage as detailed in the hrnProtocolRemoteShimInstall() documentation.
***********************************************************************************************************************************/
static void
protocolRemoteExec(
    ProtocolHelperClient *const helper, const ProtocolStorageType protocolStorageType, const unsigned int hostIdx,
    const unsigned int processId)
{
    // Call the shim when installed
    if (hrnProtocolStatic.remoteShim)
    {
        FUNCTION_LOG_BEGIN(logLevelDebug);
            FUNCTION_LOG_PARAM_P(VOID, helper);
            FUNCTION_LOG_PARAM(STRING_ID, protocolStorageType);
            FUNCTION_LOG_PARAM(UINT, hostIdx);
            FUNCTION_LOG_PARAM(UINT, processId);
        FUNCTION_LOG_END();

        // Create pipes to communicate with the subprocess. The names of the pipes are from the perspective of the parent process
        // since the child process will use them only briefly before exec'ing.
        int pipeRead[2];
        int pipeWrite[2];

        THROW_ON_SYS_ERROR(pipe(pipeRead) == -1, KernelError, "unable to create read pipe");
        THROW_ON_SYS_ERROR(pipe(pipeWrite) == -1, KernelError, "unable to create write pipe");

        // Exec command in the child process
        if (forkSafe() == 0)
        {
            // Cleanup inherited clients
            hrnProtocolClientCleanup();

            // Load configuration
            StringList *const paramList = protocolRemoteParam(protocolStorageType, hostIdx);
            hrnCfgLoadP(CFG_COMMAND_TOTAL, paramList, .noStd = true);

            // Change log process id to aid in debugging
            hrnLogProcessIdSet(processId);

            // Run server with provided handlers
            String *name = strNewFmt(PROTOCOL_SERVICE_REMOTE "-shim-%u", processId);
            ProtocolServer *server = protocolServerNew(
                name, PROTOCOL_SERVICE_REMOTE_STR, ioFdReadNewOpen(name, pipeWrite[0], 10000),
                ioFdWriteNewOpen(name, pipeRead[1], 10000));
            protocolServerProcess(server, NULL, hrnProtocolStatic.remoteHandlerList);

            // Put an end message here to sync with the client to ensure that coverage data is written before exiting
            protocolServerResponseP(server);

            // Exit when done
            exit(0);
        }

        // Close the unused file descriptors
        close(pipeRead[1]);
        close(pipeWrite[0]);

        // Create protocol object
        helper->client = protocolClientNew(
            strNewFmt(PROTOCOL_SERVICE_REMOTE "-%u shim protocol", processId), PROTOCOL_SERVICE_REMOTE_STR,
            ioFdReadNewOpen(strNewFmt(PROTOCOL_SERVICE_REMOTE "-%u shim protocol read", processId), pipeRead[0], 10000),
            ioFdWriteNewOpen(strNewFmt(PROTOCOL_SERVICE_REMOTE "-%u shim protocol write", processId), pipeWrite[1], 10000));

        // Send one noop to catch any errors that might happen after the greeting
        protocolClientNoOp(helper->client);

        FUNCTION_LOG_RETURN_VOID();
    }
    // Else call the base function
    else
        protocolRemoteExec_SHIMMED(helper, protocolStorageType, hostIdx, processId);
}

/**********************************************************************************************************************************/
void
hrnProtocolRemoteShimInstall(const List *const handlerList)
{
    FUNCTION_HARNESS_BEGIN();
        FUNCTION_HARNESS_PARAM(LIST, handlerList);
    FUNCTION_HARNESS_END();

    hrnProtocolStatic.remoteShim = true;
    hrnProtocolStatic.remoteHandlerList = handlerList;

    FUNCTION_HARNESS_RETURN_VOID();
}

/**********************************************************************************************************************************/
void
hrnProtocolRemoteShimUninstall(void)
{
    FUNCTION_HARNESS_VOID();

    hrnProtocolStatic.remoteShim = false;

    FUNCTION_HARNESS_RETURN_VOID();
}
