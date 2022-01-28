# -*- coding: utf-8 -*-
import names
import os
import sys
from os import listdir
from os.path import isfile, join, isdir
import re
import urllib.request
import json
import requests
import shutil

from objectmaphelper import RegularExpression
from pageObjects.AccountConnectionWizard import AccountConnectionWizard
from helpers.SetupClientHelper import *
from helpers.FilesHelper import buildConflictedRegex
from pageObjects.EnterPassword import EnterPassword
from pageObjects.PublicLinkDialog import PublicLinkDialog
from pageObjects.SharingDialog import SharingDialog
from pageObjects.SyncWizard import SyncWizard
from pageObjects.Toolbar import Toolbar
from pageObjects.Activity import Activity
from pageObjects.AccountStatus import AccountStatus

# the script needs to use the system wide python
# to switch from the built-in interpreter see https://kb.froglogic.com/squish/howto/using-external-python-interpreter-squish-6-6/
# if the IDE fails to reference the script, add the folder in Edit->Preferences->PyDev->Interpreters->Libraries
sys.path.append(os.path.realpath('../../../shell_integration/nautilus/'))
import syncstate
import functools


confdir = '/tmp/bdd-tests-owncloud-client/'
confFilePath = confdir + 'owncloud.cfg'
socketConnect = None

stateDataFromMiddleware = None


def getTestStateFromMiddleware(context):
    global stateDataFromMiddleware
    if stateDataFromMiddleware is None:
        res = requests.get(
            os.path.join(context.userData['middlewareUrl'], 'state'),
            headers={"Content-Type": "application/json"},
        )
        try:
            stateDataFromMiddleware = res.json()
        except ValueError:
            raise Exception("Could not get created users information from middleware")

    return stateDataFromMiddleware


@OnScenarioStart
def hook(context):
    try:
        os.makedirs(confdir, 0o0755)
    except:
        pass
    try:
        os.remove(confFilePath)
    except:
        pass


@Given(r'the user has added (the first|another) account with', regexp=True)
def step(context, accountType):
    newAccount = AccountConnectionWizard()
    if accountType == 'another':
        toolbar = Toolbar()
        toolbar.clickAddAccount()

    newAccount.addAccount(context)


@When('the user adds the following wrong user credentials:')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.addUserCreds(context)


@Then('an account should be displayed with the displayname |any| and host |any|')
def step(context, displayname, host):
    displayname = substituteInLineCodes(context, displayname)
    host = substituteInLineCodes(context, host)
    accountStatus = AccountStatus(context, displayname, host)

    test.compare(
        accountStatus.getText(),
        displayname + "\n" + host,
    )


def getDisplaynameForUser(context, username):
    usersDataFromMiddleware = getTestStateFromMiddleware(context)
    return usersDataFromMiddleware['created_users'][username]['displayname']


def getPasswordForUser(context, username):
    usersDataFromMiddleware = getTestStateFromMiddleware(context)
    return usersDataFromMiddleware['created_users'][username]['password']


@Given('user "|any|" has set up a client with default settings')
def step(context, username):
    password = getPasswordForUser(context, username)
    displayName = getDisplaynameForUser(context, username)
    setUpClient(context, username, displayName, confFilePath)
    enterUserPassword = EnterPassword()
    enterUserPassword.enterPassword(password)


@Given('the user has started the client')
def step(context):
    startClient(context)


@When(r'the user adds (the first|another) account with', regexp=True)
def step(context, accountType):
    newAccount = AccountConnectionWizard()
    if accountType == 'another':
        toolbar = Toolbar()
        toolbar.clickAddAccount()

    newAccount.addAccount(context)


@Given('the user has added the following account information:')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.addServer(context)
    newAccount.addUserCreds(context)
    newAccount.selectSyncFolder(context)


def isItemSynced(type, itemName):
    if type != 'FILE' and type != 'FOLDER':
        raise Exception("type must be 'FILE' or 'FOLDER'")
    socketConnect = syncstate.SocketConnect()
    socketConnect.sendCommand("RETRIEVE_" + type + "_STATUS:" + itemName + "\n")

    if not socketConnect.read_socket_data_with_timeout(0.1):
        return False
    for line in socketConnect.get_available_responses():
        if line.startswith('STATUS:OK') and line.endswith(itemName):
            return True
        elif line.endswith(itemName):
            return False


def isFolderSynced(folderName):
    return isItemSynced('FOLDER', folderName)


def isFileSynced(fileName):
    return isItemSynced('FILE', fileName)


def waitForFileToBeSynced(context, filePath):
    waitFor(
        lambda: isFileSynced(sanitizePath(filePath)),
        context.userData['clientSyncTimeout'] * 1000,
    )


def waitForFolderToBeSynced(context, folderPath):
    waitFor(
        lambda: isFolderSynced(sanitizePath(folderPath)),
        context.userData['clientSyncTimeout'] * 1000,
    )


def folderExists(folderPath, timeout=1000):
    return waitFor(
        lambda: isdir(sanitizePath(folderPath)),
        timeout,
    )


def fileExists(filePath, timeout=1000):
    return waitFor(
        lambda: isfile(sanitizePath(filePath)),
        timeout,
    )


def sanitizePath(path):
    return path.replace('//', '/')


def shareResource(resource):
    socketConnect = syncstate.SocketConnect()
    socketConnect.sendCommand("SHARE:" + resource + "\n")
    if not socketConnect.read_socket_data_with_timeout(0.1):
        return False
    for line in socketConnect.get_available_responses():
        if line.startswith('SHARE:OK') and line.endswith(resource):
            return True
        elif line.endswith(resource):
            return False


def executeStepThroughMiddleware(context, step):
    body = {"step": step}
    if hasattr(context, "table"):
        body["table"] = context.table

    params = json.dumps(body).encode('utf8')

    req = urllib.request.Request(
        context.userData['middlewareUrl'] + 'execute',
        data=params,
        headers={"Content-Type": "application/json"},
        method='POST',
    )
    try:
        urllib.request.urlopen(req)
    except urllib.error.HTTPError as e:
        raise Exception(
            "Step execution through test middleware failed. Error: " + e.read().decode()
        )


@When(
    'the user adds "|any|" as collaborator of resource "|any|" with permissions "|any|" using the client-UI'
)
def step(context, receiver, resource, permissions):
    openSharingDialog(context, resource)
    shareItem = SharingDialog()
    shareItem.addCollaborator(receiver, permissions)
    shareItem.closeSharingDialog()


@When('the user adds following collaborators of resource "|any|" using the client-UI')
def step(context, resource):
    openSharingDialog(context, resource)
    shareItem = SharingDialog()

    # In the following loop we are trying to share resource with given permission to one user at a time given from the data table in the feature file
    for row in context.table[1:]:
        receiver = row[0]
        permissions = row[1]
        shareItem.addCollaborator(receiver, permissions)

    shareItem.closeSharingDialog()


@When(
    'the user selects "|any|" as collaborator of resource "|any|" using the client-UI'
)
def step(context, receiver, resource):
    openSharingDialog(context, resource)
    shareItem = SharingDialog()
    shareItem.selectCollaborator(receiver)


@When(
    'the user adds group "|any|" as collaborator of resource "|any|" with permissions "|any|" using the client-UI'
)
def step(context, receiver, resource, permissions):
    openSharingDialog(context, resource)
    shareItem = SharingDialog()
    shareItem.addCollaborator(receiver, permissions, True)
    shareItem.closeSharingDialog()


@Then(
    'user "|any|" should be listed in the collaborators list for file "|any|" with permissions "|any|" on the client-UI'
)
def step(context, receiver, resource, permissions):
    collaboratorShouldBeListed(context, receiver, resource, permissions)


@Then(
    'group "|any|" should be listed in the collaborators list for file "|any|" with permissions "|any|" on the client-UI'
)
def step(context, receiver, resource, permissions):
    receiver += " (group)"
    collaboratorShouldBeListed(context, receiver, resource, permissions)


def collaboratorShouldBeListed(
    context, receiver, resource, permissions, receiverCount=0
):
    resource = getResourcePath(context, resource)
    socketConnect = syncstate.SocketConnect()
    socketConnect.sendCommand("SHARE:" + resource + "\n")
    permissionsList = permissions.split(',')

    waitForObject(
        {
            "container": names.sharingDialogUG_scrollArea_QScrollArea,
            "name": "sharedWith",
            "type": "QLabel",
            "visible": 1,
        }
    )

    # findAllObjects: This functionfinds and returns a list of object references identified by the symbolic or real (multi-property) name objectName.
    sharedWithObj = findAllObjects(
        {
            "container": names.sharingDialogUG_scrollArea_QScrollArea,
            "name": "sharedWith",
            "type": "QLabel",
            "visible": 1,
        }
    )

    #     we use sharedWithObj list from above while verifying if users are listed or not.
    #     For this we need an index value i.e receiverCount
    #     For 1st user in the list the index will be 0 which is receiverCount default value
    #     For 2nd user in the list the index will be 1 and so on

    test.compare(str(sharedWithObj[receiverCount].text), receiver)
    test.compare(
        waitForObjectExists(names.scrollArea_permissionsEdit_QCheckBox).checked,
        ('edit' in permissionsList),
    )
    test.compare(
        waitForObjectExists(names.scrollArea_permissionShare_QCheckBox).checked,
        ('share' in permissionsList),
    )
    shareItem = SharingDialog()
    shareItem.closeSharingDialog()


@When('the user waits for the files to sync')
def step(context):
    waitForFolderToBeSynced(context, '/')


def waitForResourceToSync(context, resource, resourceType):
    resource = join(context.userData['currentUserSyncPath'], resource)
    if resourceType == "file":
        waitForFileToBeSynced(context, resource)
    elif resourceType == "folder":
        waitForFolderToBeSynced(context, resource)


@When(r'the user waits for (file|folder) "([^"]*)" to be synced', regexp=True)
def step(context, type, resource):
    waitForResourceToSync(context, resource, type)


@Given(r'the user has waited for (file|folder) "([^"]*)" to be synced', regexp=True)
def step(context, type, resource):
    waitForResourceToSync(context, resource, type)


@Given(
    'user "|any|" has created a file "|any|" with the following content inside the sync folder'
)
def step(context, username, filename):
    createFile(context, filename, username)


@When(
    'user "|any|" creates a file "|any|" with the following content inside the sync folder'
)
def step(context, username, filename):
    createFile(context, filename, username)


def createFile(context, filename, username=None):
    fileContent = "\n".join(context.multiLineText)
    syncPath = None
    if username:
        syncPath = getUserSyncPath(context, username)
    else:
        syncPath = context.userData['currentUserSyncPath']

    # A file is scheduled to be synced but is marked as ignored for 5 seconds. And if we try to sync it, it will fail. So we need to wait for 5 seconds.
    # https://github.com/owncloud/client/issues/9325
    snooze(5)

    f = open(join(syncPath, filename), "w")
    f.write(fileContent)
    f.close()


@When('user "|any|" creates a folder "|any|" inside the sync folder')
def step(context, username, foldername):
    createFolder(context, foldername, username)


@Given('user "|any|" has created a folder "|any|" inside the sync folder')
def step(context, username, foldername):
    createFolder(context, foldername, username)


def createFolder(context, foldername, username=None):
    syncPath = None
    if username:
        syncPath = getUserSyncPath(context, username)
    else:
        syncPath = context.userData['currentUserSyncPath']
    path = join(syncPath, foldername)
    os.makedirs(path)


@When('the user copies the folder "|any|" to "|any|"')
def step(context, sourceFolder, destinationFolder):
    source_dir = join(context.userData['currentUserSyncPath'], sourceFolder)
    destination_dir = join(context.userData['currentUserSyncPath'], destinationFolder)
    shutil.copytree(source_dir, destination_dir)


@Given(r"^(.*) on the server (.*)$", regexp=True)
def step(context, stepPart1, stepPart2):
    executeStepThroughMiddleware(context, "Given " + stepPart1 + " " + stepPart2)
    global usersDataFromMiddleware
    usersDataFromMiddleware = None


@Given(r"^(.*) on the server$", regexp=True)
def step(context, stepPart1):
    executeStepThroughMiddleware(context, "Given " + stepPart1)
    global usersDataFromMiddleware
    usersDataFromMiddleware = None


@Then(r"^(.*) on the server (.*)$", regexp=True)
def step(context, stepPart1, stepPart2):
    executeStepThroughMiddleware(context, "Then " + stepPart1 + " " + stepPart2)


@Then(r"^(.*) on the server$", regexp=True)
def step(context, stepPart1):
    executeStepThroughMiddleware(context, "Then " + stepPart1)


@Then('the file "|any|" should exist on the file system with the following content')
def step(context, filePath):
    expected = "\n".join(context.multiLineText)
    filePath = context.userData['currentUserSyncPath'] + filePath
    f = open(filePath, 'r')
    contents = f.read()
    test.compare(
        expected,
        contents,
        "file expected to exist with content "
        + expected
        + " but does not have the expected content",
    )


@Then(r'^the (file|folder) "([^"]*)" should exist on the file system$', regexp=True)
def step(context, resourceType, resource):
    resourcePath = join(context.userData['currentUserSyncPath'], resource)
    resourceExists = False
    if resourceType == 'file':
        resourceExists = fileExists(
            resourcePath, context.userData['clientSyncTimeout'] * 1000
        )
    elif resourceType == 'folder':
        resourceExists = folderExists(
            resourcePath, context.userData['clientSyncTimeout'] * 1000
        )
    else:
        raise Exception("Unsupported resource type '" + resourceType + "'")

    test.compare(
        True,
        resourceExists,
        "Assert " + resourceType + " '" + resource + "' exists on the system",
    )


@Then(r'^the (file|folder) "([^"]*)" should not exist on the file system$', regexp=True)
def step(context, resourceType, resource):
    resourcePath = join(context.userData['currentUserSyncPath'], resource)
    resourceExists = False
    if resourceType == 'file':
        resourceExists = fileExists(resourcePath, 1000)
    elif resourceType == 'folder':
        resourceExists = folderExists(resourcePath, 1000)
    else:
        raise Exception("Unsupported resource type '" + resourceType + "'")

    test.compare(
        False,
        resourceExists,
        "Assert " + resourceType + " '" + resource + "' doesn't exist on the system",
    )


@Given('the user has paused the file sync')
def step(context):
    waitForFolderToBeSynced(context, '/')
    syncWizard = SyncWizard()
    syncWizard.performAction("Pause sync")


@Given('the user has changed the content of local file "|any|" to:')
def step(context, filename):
    fileContent = "\n".join(context.multiLineText)
    f = open(context.userData['currentUserSyncPath'] + filename, "w")
    f.write(fileContent)
    f.close()


@When('the user resumes the file sync on the client')
def step(context):
    syncWizard = SyncWizard()
    syncWizard.performAction("Resume sync")


@When('the user triggers force sync on the client')
def step(context):
    mouseClick(
        waitForObjectItem(names.stack_folderList_QTreeView, "_1"),
        720,
        36,
        Qt.NoModifier,
        Qt.LeftButton,
    )
    activateItem(waitForObjectItem(names.settings_QMenu, "Force sync now"))


@Then(
    'a conflict file for "|any|" should exist on the file system with the following content'
)
def step(context, filename):
    expected = "\n".join(context.multiLineText)

    namepart = filename.split('.')[0]
    extpart = filename.split('.')[1]
    onlyfiles = [
        f
        for f in listdir(context.userData['currentUserSyncPath'])
        if isfile(join(context.userData['currentUserSyncPath'], f))
    ]
    found = False
    pattern = re.compile(buildConflictedRegex(filename))
    for file in onlyfiles:
        if pattern.match(file):
            f = open(context.userData['currentUserSyncPath'] + file, 'r')
            contents = f.read()
            if contents == expected:
                found = True
                break

    if not found:
        raise Exception("Conflict file not found with given name")


@When('the user clicks on the activity tab')
def step(context):
    toolbar = Toolbar()
    toolbar.clickActivity()


@Then('a conflict warning should be shown for |integer| files')
def step(context, files):
    clickTab(waitForObject(names.stack_QTabWidget), "Not Synced ({})".format(files))
    test.compare(
        waitForObjectExists(
            names.oCC_IssuesWidget_treeWidget_QTreeWidget
        ).topLevelItemCount,
        files,
    )
    test.compare(
        waitForObjectExists(names.oCC_IssuesWidget_treeWidget_QTreeWidget).visible, True
    )
    test.compare(
        waitForObjectExists(
            names.o_treeWidget_Conflict_Server_version_downloaded_local_copy_renamed_and_not_uploaded_QModelIndex
        ).displayText,
        "Conflict: Server version downloaded, local copy renamed and not uploaded.",
    )


@Then('the table of conflict warnings should include file "|any|"')
def step(context, filename):
    activity = Activity()
    activity.checkFileExist(filename)


@Then('the file "|any|" should be blacklisted')
def step(context, filename):
    activity = Activity()
    activity.checkBlackListedFileExist(filename)


@When('the user selects "|any|" tab in the activity')
def step(context, tabName):
    activity = Activity()
    activity.clickTab(tabName)


def openSharingDialog(context, resource, itemType='file'):
    resource = getResourcePath(context, resource)

    if itemType == 'folder':
        waitFor(
            lambda: isFolderSynced(resource),
            context.userData['clientSyncTimeout'] * 1000,
        )
    elif itemType == 'file':
        waitFor(
            lambda: isFileSynced(resource), context.userData['clientSyncTimeout'] * 1000
        )
    else:
        raise Exception("No such item type for resource")

    waitFor(
        lambda: shareResource(resource), context.userData['clientSyncTimeout'] * 1000
    )


@When('the user opens the public links dialog of "|any|" using the client-UI')
def step(context, resource):
    openSharingDialog(context, resource)
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.openPublicLinkDialog()


@When("the user toggles the password protection using the client-UI")
def step(context):
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.togglePassword()


@Then('the password progress indicator should not be visible in the client-UI')
def step(context):
    waitFor(lambda: (test.vp("publicLinkPasswordProgressIndicatorInvisible")))


@Then(
    'the password progress indicator should not be visible in the client-UI - expected to fail'
)
def step(context):
    waitFor(lambda: (test.xvp("publicLinkPasswordProgressIndicatorInvisible")))


@When('the user opens the sharing dialog of "|any|" using the client-UI')
def step(context, resource):
    openSharingDialog(context, resource, 'folder')


def getSharingDialogText():
    shareItem = SharingDialog()
    errorText = shareItem.getSharingDialogMessage()
    return errorText


@Then('the text "|any|" should be displayed in the sharing dialog')
def step(context, fileShareContext):
    errorText = getSharingDialogText()
    test.compare(
        errorText,
        fileShareContext,
    )


@Then('the error text "|any|" should be displayed in the sharing dialog')
def step(context, fileShareContext):
    errorText = getSharingDialogText()
    test.compare(
        errorText,
        fileShareContext,
    )


def createPublicLinkShare(
    context, resource, password='', permissions='', expireDate='', name=''
):
    resource = getResourcePath(context, resource)
    openSharingDialog(context, resource)
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.openPublicLinkDialog()
    publicLinkDialog.createPublicLink(
        context, resource, password, permissions, expireDate, name
    )


@When(
    'the user creates a new public link for file "|any|" without password using the client-UI'
)
def step(context, resource):
    createPublicLinkShare(context, resource)


@When(
    'the user creates a new public link for file "|any|" with password "|any|" using the client-UI'
)
def step(context, resource, password):
    createPublicLinkShare(context, resource, password)


def setExpirationDateWithVerification(resource, publicLinkName, expireDate):
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.verifyResource(resource)
    publicLinkDialog.verifyPublicLinkName(publicLinkName)
    publicLinkDialog.setExpirationDate(expireDate)


@When('the user edits the public link named "|any|" of file "|any|" changing following')
def step(context, publicLinkName, resource):
    expireDate = ''
    for row in context.table:
        if row[0] == 'expireDate':
            expireDate = row[1]
            break
    setExpirationDateWithVerification(resource, publicLinkName, expireDate)


@When(
    'the user creates a new public link with permissions "|any|" for folder "|any|" without password using the client-UI'
)
def step(context, permissions, resource):
    createPublicLinkShare(context, resource, '', permissions)


@When(
    'the user creates a new public link with permissions "|any|" for folder "|any|" with password "|any|" using the client-UI'
)
def step(context, permissions, resource, password):
    createPublicLinkShare(context, resource, password, permissions)


@When('the user creates a new public link with following settings using the client-UI:')
def step(context):
    linkSettings = {}
    for row in context.table:
        linkSettings[row[0]] = row[1]
    createPublicLinkShare(
        context,
        resource=linkSettings['path'],
        password=linkSettings['password'],
        expireDate=linkSettings['expireDate'],
    )


def createPublicShareWithRole(context, resource, role):
    resource = sanitizePath(substituteInLineCodes(context, resource))
    openSharingDialog(context, resource)
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.openPublicLinkDialog()
    publicLinkDialog.createPublicLinkWithRole(role)


@When(
    'the user creates a new public link for folder "|any|" using the client-UI with these details:'
)
def step(context, resource):
    role = ''
    for row in context.table:
        if row[0] == 'role':
            role = row[1]
            break

    if role == '':
        raise Exception("No role has been found")
    else:
        createPublicShareWithRole(context, resource, role)


@When('the user "|any|" logs out of the client-UI')
def step(context, username):
    accountStatus = AccountStatus(context, getDisplaynameForUser(context, username))
    accountStatus.accountAction("Log out")


def isUserSignedOut(context, username):
    displayname = getDisplaynameForUser(context, username)
    server = context.userData['localBackendUrl']
    accountStatus = AccountStatus(context, getDisplaynameForUser(context, username))
    test.compare(
        str(waitForObjectExists(accountStatus.SIGNED_OUT_TEXT_BAR).text),
        'Signed out from <a href="'
        + server
        + '">'
        + server
        + '</a> as <i>'
        + displayname
        + '</i>.',
    )


def isUserSignedIn(context, username):
    displayname = getDisplaynameForUser(context, username)
    server = context.userData['localBackendUrl']
    accountStatus = AccountStatus(context, getDisplaynameForUser(context, username))
    test.compare(
        str(waitForObjectExists(accountStatus.SIGNED_OUT_TEXT_BAR).text),
        'Connected '
        + 'to <a href="'
        + server
        + '">'
        + server
        + '</a> as <i>'
        + displayname
        + '</i>.',
    )


@Then('user "|any|" should be signed out')
def step(context, username):
    isUserSignedOut(context, username)


@Given('user "|any|" has logged out of the client-UI')
def step(context, username):
    waitForFolderToBeSynced(context, '/')
    # TODO: find some way to dynamically to check if files are synced
    # It might take some time for all files to sync
    snooze(5)
    accountStatus = AccountStatus(context, getDisplaynameForUser(context, username))
    accountStatus.accountAction("Log out")
    isUserSignedOut(context, username)


@When('user "|any|" logs in to the client-UI')
def step(context, username):
    accountStatus = AccountStatus(context, getDisplaynameForUser(context, username))
    accountStatus.accountAction("Log in")
    password = getPasswordForUser(context, username)
    enterUserPassword = EnterPassword()
    enterUserPassword.enterPassword(password)


@Then('user "|any|" should be connect to the client-UI')
def step(context, username):
    # TODO: find some way to dynamically to check if files are synced
    # It might take some time for all files to sync and connect to ther server
    snooze(5)
    isUserSignedIn(context, username)


@When('the user removes the connection for user "|any|" and host |any|')
def step(context, username, host):
    displayname = getDisplaynameForUser(context, username)
    displayname = substituteInLineCodes(context, displayname)
    host = substituteInLineCodes(context, host)

    waitForFolderToBeSynced(context, '/')
    accountStatus = AccountStatus(context, displayname, host)
    accountStatus.removeConnection()


@Then('an account with the displayname |any| and host |any| should not be displayed')
def step(context, displayname, host):
    displayname = substituteInLineCodes(context, displayname)
    host = substituteInLineCodes(context, host)
    toolbar = Toolbar()
    displayedAccountText = toolbar.getDisplayedAccountText(displayname, host)

    test.compare(
        displayedAccountText,
        displayname + "\n" + host,
    )


@Then('connection wizard should be visible')
def step(context):
    test.compare(
        str(waitForObjectExists(names.owncloudWizard_label_2_QLabel).text),
        'Ser&ver Address',
    )
    waitForObject(AccountConnectionWizard.SERVER_ADDRESS_BOX)


@Then("the following tabs in the toolbar should match the default baseline")
def step(context):
    for tabName in context.table:
        test.vp(tabName[0])


@When(
    'the user removes permissions "|any|" for user "|any|" of resource "|any|" using the client-UI'
)
def step(context, permissions, receiver, resource):
    openSharingDialog(context, resource)
    test.compare(
        str(waitForObjectExists(names.scrollArea_sharedWith_QLabel).text), receiver
    )

    shareItem = SharingDialog()
    shareItem.removePermissions(permissions)


@When("the user closes the sharing dialog")
def step(context):
    clickButton(waitForObject(names.sharingDialog_Close_QPushButton))


@Then(
    '"|any|" permissions should not be displayed for user "|any|" for resource "|any|" on the client-UI'
)
def step(context, permissions, user, resource):
    permissionsList = permissions.split(',')

    shareItem = SharingDialog()
    shareItem.verifyResource(resource)
    editChecked, shareChecked = shareItem.getAvailablePermission()

    if 'edit' in permissionsList:
        test.compare(editChecked, False)

    if 'share' in permissionsList:
        test.compare(shareChecked, False)


@Then('the error "|any|" should be displayed')
def step(context, errorMessage):
    sharingDialog = SharingDialog()
    test.compare(sharingDialog.getErrorText(), errorMessage)


@When(
    'the user tires to share resource "|any|" with the group "|any|" using the client-UI'
)
def step(context, resource, group):
    openSharingDialog(context, resource)

    sharingDialog = SharingDialog()
    sharingDialog.selectCollaborator(group, True)


def overwriteFile(resource, content):
    f = open(resource, "w")
    f.write(content)
    f.close()


def tryToOverwriteFile(context, resource, content):
    waitForFileToBeSynced(context, resource)
    waitForFolderToBeSynced(context, '/')

    try:
        overwriteFile(resource, content)
    except:
        pass

    waitForFileToBeSynced(context, resource)


@When('the user overwrites the file "|any|" with content "|any|"')
def step(context, resource, content):
    print("starting file overwrite")
    resource = join(context.userData['currentUserSyncPath'], resource)
    waitForFileToBeSynced(context, resource)
    waitForFolderToBeSynced(context, '/')

    # overwriting the file immediately after it has been synced from the server seems to have some problem.
    # The client does not see the change although the changes have already been made thus we are having a race condition
    # So for now we add 5sec static wait
    # an issue https://github.com/owncloud/client/issues/8832 has been created for it

    snooze(5)

    overwriteFile(resource, content)

    print("file has been overwritten")
    waitForFileToBeSynced(context, resource)


@When('the user tries to overwrite the file "|any|" with content "|any|"')
def step(context, resource, content):
    resource = context.userData['currentUserSyncPath'] + resource
    tryToOverwriteFile(context, resource, content)


@When('user "|any|" tries to overwrite the file "|any|" with content "|any|"')
def step(context, user, resource, content):
    resource = getResourcePath(context, resource, user)
    tryToOverwriteFile(context, resource, content)


def enableVFSSupport(vfsBtnText):
    # The enabling/disabling VFS button do not have it's own object
    # But it is inside the "stack_folderList_QTreeView" object.
    # So we are clicking at (718, 27) of "stack_folderList_QTreeView" object to enable/disable VFS
    mouseClick(
        waitForObjectItem(names.stack_folderList_QTreeView, "_1"),
        718,
        27,
        Qt.NoModifier,
        Qt.LeftButton,
    )
    activateItem(waitForObjectItem(names.settings_QMenu, vfsBtnText))
    clickButton(
        waitForObject(names.stack_Enable_experimental_placeholder_mode_QPushButton)
    )


@When("the user enables virtual file support")
def step(context):
    enableVFSSupport("Enable virtual file support (experimental)...")


@Then('the "|any|" button should be available')
def step(context, btnText):
    # The enabling/disabling VFS button do not have it's own object
    # But it is inside the "stack_folderList_QTreeView" object.
    # So we are clicking at (718, 27) of "stack_folderList_QTreeView" object to enable/disable VFS
    mouseClick(
        waitForObjectItem(names.stack_folderList_QTreeView, "_1"),
        718,
        27,
        Qt.NoModifier,
        Qt.LeftButton,
    )
    waitForObjectItem(names.settings_QMenu, btnText)


@Given("the user has enabled virtual file support")
def step(context):
    enableVFSSupport("Enable virtual file support (experimental)...")


@When("the user disables virtual file support")
def step(context):
    # The enabling/disabling VFS button do not have it's own object
    # But it is inside the "stack_folderList_QTreeView" object.
    # So we are clicking at (718, 27) of "stack_folderList_QTreeView" object to enable/disable VFS
    mouseClick(
        waitForObjectItem(names.stack_folderList_QTreeView, "_1"),
        733,
        27,
        Qt.NoModifier,
        Qt.LeftButton,
    )
    activateItem(
        waitForObjectItem(names.settings_QMenu, "Disable virtual file support...")
    )
    clickButton(
        waitForObject(names.disable_virtual_file_support_Disable_support_QPushButton)
    )


@When('the user accepts the certificate')
def step(context):
    clickButton(waitForObject(names.oCC_SslErrorDialog_cbTrustConnect_QCheckBox))
    clickButton(waitForObject(names.oCC_SslErrorDialog_OK_QPushButton))


@Then('the lock shown should be closed')
def step(context):
    test.vp("urlLock")


@Then('error "|any|" should be displayed')
def step(context, errorMsg):
    newAccount = AccountConnectionWizard()
    test.compare(str(waitForObjectExists(newAccount.ERROR_LABEL).text), errorMsg)


@When(r'the user deletes the (file|folder) "([^"]*)"', regexp=True)
def step(context, itemType, resource):
    resourcePath = sanitizePath(context.userData['currentUserSyncPath'] + resource)
    if itemType == 'file':
        os.remove(resourcePath)
    elif itemType == 'folder':
        shutil.rmtree(resourcePath)
    else:
        raise Exception("No such item type for resource")


@When(
    'the user unshares the resource "|any|" for collaborator "|any|" using the client-UI'
)
def step(context, resource, receiver):
    openSharingDialog(context, resource)
    test.compare(
        str(waitForObjectExists(names.scrollArea_sharedWith_QLabel).text), receiver
    )
    clickButton(waitForObject(names.scrollArea_deleteShareButton_QToolButton))


@Given('the user has added the following server address:')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.addServer(context)
    test.compare(
        waitForObjectExists(newAccount.CREDENTIAL_PAGE).visible,
        True,
        "Assert credentials page is visible",
    )


@When('the user adds the following server address:')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.addServer(context)


@Given('the user has added the following user credentials:')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.addUserCreds(context)
    test.compare(
        waitForObjectExists(newAccount.ADVANCE_SETUP_PAGE).visible,
        True,
        "Assert setup page is visible",
    )


@Given('the user has opened chose_what_to_sync dialog')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.openSyncDialog()
    test.compare(
        waitForObjectExists(newAccount.SELECTIVE_SYNC_DIALOG).visible,
        True,
        "Assert selective sync dialog is visible",
    )


@When('the user opens chose_what_to_sync dialog')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.openSyncDialog()


@When('the user selects the following folders to sync:')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.selectFoldersToSync(context)


@When('the user selects manual sync folder option')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.selectManualSyncFolder()


@When('the user connects the account')
def step(context):
    newAccount = AccountConnectionWizard()
    newAccount.connectAccount()


@When('the user sorts the folder list by "|any|"')
def step(context, headerText):
    headerText = headerText.capitalize()
    if headerText in ["Size", "Name"]:
        newAccount = AccountConnectionWizard()
        newAccount.sortBy(headerText)
    else:
        raise Exception("Sorting by '" + headerText + "' is not supported.")


@Then('the dialog chose_what_to_sync should be visible')
def step(context):
    newAccount = AccountConnectionWizard()
    test.compare(
        waitForObjectExists(newAccount.SELECTIVE_SYNC_DIALOG).visible,
        True,
        "Assert selective sync dialog is visible",
    )


@Then('the sync all checkbox should be checked')
def step(context):
    newAccount = AccountConnectionWizard()
    test.compare(
        waitForObjectExists(newAccount.SYNC_DIALOG_ROOT_FOLDER).checkState,
        "checked",
        "Assert sync all checkbox is checked",
    )


@Then("the folders should be in the following order:")
def step(context):
    newAccount = AccountConnectionWizard()
    rowIndex = 0
    for row in context.table[1:]:
        FOLDER_TREE_ROW = {
            "row": rowIndex,
            "container": newAccount.SYNC_DIALOG_ROOT_FOLDER,
            "type": "QModelIndex",
        }
        expectedFolder = row[0]
        actualFolder = waitForObjectExists(FOLDER_TREE_ROW).displayText
        test.compare(actualFolder, expectedFolder)

        rowIndex += 1


@When('the user deletes the public link for file "|any|"')
def step(context, resource):
    openSharingDialog(context, resource)
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.openPublicLinkDialog()

    test.compare(
        str(waitForObjectExists(publicLinkDialog.ITEM_TO_SHARE).text),
        resource.replace(context.userData['currentUserSyncPath'], ''),
    )
    clickButton(waitForObject(names.linkShares_QToolButton_2))
    clickButton(waitForObject(names.oCC_ShareLinkWidget_Delete_QPushButton))

    waitFor(
        lambda: (not object.exists(names.linkShares_QToolButton_2)),
    )


@When(
    'the user changes the password of public link "|any|" to "|any|" using the client-UI'
)
def step(context, publicLinkName, password):
    publicLinkDialog = PublicLinkDialog()
    publicLinkDialog.verifyPublicLinkName(publicLinkName)
    publicLinkDialog.changePassword(password)


@Then(
    'the following users should be listed in as collaborators for file "|any|" on the client-UI'
)
def step(context, resource):
    #     Here we are trying to verify if the user added in when step are listed in the client-UI or not
    #     We now have a variable name receiverCount which is used in collaboratorShouldBeListed function call
    receiverCount = 0
    for row in context.table[1:]:
        receiver = row[0]
        permissions = row[1]

        collaboratorShouldBeListed(
            context, receiver, resource, permissions, receiverCount
        )
        receiverCount += 1


def searchCollaborator(collaborator):
    shareItem = SharingDialog()
    shareItem.searchCollaborator(collaborator)


@When('the user searches for collaborator "|any|" using the client-UI')
def step(context, collaborator):
    searchCollaborator(collaborator)


@When(
    'the user searches for collaborator with autocomplete characters "|any|" using the client-UI'
)
def step(context, collaborator):
    searchCollaborator(collaborator)


@Then('the following users should be listed as suggested collaborators:')
def step(context):
    shareItem = SharingDialog()
    for collaborator in context.table[1:]:
        exists = False
        try:
            waitForObjectItem(shareItem.SUGGESTED_COLLABORATOR, collaborator[0])
            exists = True
        except LookupError as e:
            pass

        test.compare(exists, True, "Assert user '" + collaborator[0] + "' is listed")


@Then('the collaborators should be listed in the following order:')
def step(context):
    shareItem = SharingDialog()
    for index, collaborator in enumerate(context.table[1:], start=1):
        test.compare(
            str(
                waitForObjectExists(
                    {
                        "container": names.sharingDialogUG_scrollArea_QScrollArea,
                        "name": "sharedWith",
                        "occurrence": index,
                        "type": "QLabel",
                        "visible": 1,
                    }
                ).text
            ),
            collaborator[0],
        )


@Then('VFS enabled baseline image should match the default screenshot')
def step(context):
    test.vp("VP_VFS_enabled")


@Then('VFS enabled baseline image should not match the default screenshot')
def step(context):
    test.xvp("VP_VFS_enabled")
