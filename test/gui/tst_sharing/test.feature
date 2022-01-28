Feature: Sharing

    As a user
    I want to share files and folders with other users
    So that those users can access the files and folders


    Background:
        Given user "Alice" has been created on the server with default attributes and without skeleton files
        And the setting "shareapi_auto_accept_share" on the server of app "core" has been set to "yes"

    @smokeTest
    Scenario: simple sharing with user
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user adds "Brian Murphy" as collaborator of resource "textfile0.txt" with permissions "edit,share" using the client-UI
        And the user adds "Brian Murphy" as collaborator of resource "simple-folder" with permissions "edit,share" using the client-UI
        Then user "Brian Murphy" should be listed in the collaborators list for file "textfile0.txt" with permissions "edit,share" on the client-UI
        And user "Brian Murphy" should be listed in the collaborators list for file "simple-folder" with permissions "edit,share" on the client-UI
        And as "Brian" folder "simple-folder" should exist on the server
        And as "Brian" file "textfile0.txt" should exist on the server


    Scenario: sharing file and folder with user who has some other shares
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "shared" on the server
        And user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has uploaded file with content "shared file" to "sharedfile.txt" on the server
        And user "Alice" has shared folder "shared" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared file "sharedfile.txt" on the server with user "Brian" with "all" permissions
        And user "Alice" has set up a client with default settings
        When the user adds "Brian Murphy" as collaborator of resource "textfile.txt" with permissions "edit,share" using the client-UI
        And the user adds "Brian Murphy" as collaborator of resource "simple-folder" with permissions "edit,share" using the client-UI
        Then user "Brian Murphy" should be listed in the collaborators list for file "textfile.txt" with permissions "edit,share" on the client-UI
        And user "Brian Murphy" should be listed in the collaborators list for file "simple-folder" with permissions "edit,share" on the client-UI
        And as "Brian" folder "shared" should exist on the server
        And as "Brian" file "sharedfile.txt" should exist on the server
        And as "Brian" folder "simple-folder" should exist on the server
        And as "Brian" file "textfile.txt" should exist on the server


    Scenario: sharing file/folder with a user that has special characters as username
        Given user "Speci@l_Name-.+" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "FOLDER" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user opens the sharing dialog of "textfile.txt" using the client-UI
        And the user adds "Speci@l_Name-.+" as collaborator of resource "textfile.txt" with permissions "edit,share" using the client-UI
        And the user adds "Speci@l_Name-.+" as collaborator of resource "FOLDER" with permissions "edit,share" using the client-UI
        Then user "Speci@l_Name-.+" should be listed in the collaborators list for file "textfile.txt" with permissions "edit,share" on the client-UI
        And user "Speci@l_Name-.+" should be listed in the collaborators list for file "FOLDER" with permissions "edit,share" on the client-UI
        And as "Speci@l_Name-.+" folder "FOLDER" should exist on the server
        And as "Speci@l_Name-.+" file "textfile.txt" should exist on the server


    Scenario: Share files/folders with special characters in their name
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "SampleFolder,With,$pecial?Characters" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/$ample1?.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user adds "Brian Murphy" as collaborator of resource "SampleFolder,With,$pecial?Characters" with permissions "edit,share" using the client-UI
        And the user adds "Brian Murphy" as collaborator of resource "$ample1?.txt" with permissions "edit,share" using the client-UI
        Then user "Brian Murphy" should be listed in the collaborators list for file "SampleFolder,With,$pecial?Characters" with permissions "edit,share" on the client-UI
        And user "Brian Murphy" should be listed in the collaborators list for file "$ample1?.txt" with permissions "edit,share" on the client-UI
        And as "Brian" folder "SampleFolder,With,$pecial?Characters" should exist on the server
        And as "Brian" file "$ample1?.txt" should exist on the server


    Scenario: try to share a file/folder with a user to whom the file has already been shared
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "SharedFolder" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file" to "/textfile.txt" on the server
        And user "Alice" has shared folder "SharedFolder" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "all" permissions
        And user "Alice" has set up a client with default settings
        When the user opens the sharing dialog of "textfile.txt" using the client-UI
        And the user searches for collaborator "Brian Murphy" using the client-UI
        Then the error "No results for 'Brian Murphy'" should be displayed
        When the user closes the sharing dialog
        And the user opens the sharing dialog of "SharedFolder" using the client-UI
        And the user searches for collaborator "Brian Murphy" using the client-UI
        Then the error "No results for 'Brian Murphy'" should be displayed


    Scenario: try to self share a file/folder
        Given user "Alice" has uploaded file with content "ownCloud test text file" to "/textfile.txt" on the server
        And user "Alice" has created folder "OwnFolder" on the server
        And user "Alice" has set up a client with default settings
        When the user opens the sharing dialog of "textfile.txt" using the client-UI
        And the user selects "Alice Hansen" as collaborator of resource "textfile.txt" using the client-UI
        Then the error "Can't share with yourself" should be displayed
        When the user closes the sharing dialog
        And the user opens the sharing dialog of "OwnFolder" using the client-UI
        And the user selects "Alice Hansen" as collaborator of resource "OwnFolder" using the client-UI
        Then the error "Can't share with yourself" should be displayed


    Scenario: search for users with minimum autocomplete characters
        Given user "TestUser1" has been created on the server with default attributes and without skeleton files
        And user "TestUser2" has been created on the server with default attributes and without skeleton files
        And user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user opens the sharing dialog of "textfile.txt" using the client-UI
        And the user searches for collaborator with autocomplete characters "Tes" using the client-UI
        Then the following users should be listed as suggested collaborators:
            | user      |
            | TestUser1 |
            | TestUser2 |


    Scenario: collaborators are listed in chronological order
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Carol" has been created on the server with default attributes and without skeleton files
        And user "TestUser1" has been created on the server with default attributes and without skeleton files
        And user "TestUser2" has been created on the server with default attributes and without skeleton files
        And user "TestUser3" has been created on the server with default attributes and without skeleton files
        And user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has shared file "textfile.txt" on the server with user "Carol" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "TestUser1" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "TestUser3" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "TestUser2" with "all" permissions
        And user "Alice" has set up a client with default settings
        When the user opens the sharing dialog of "textfile.txt" using the client-UI
        Then the collaborators should be listed in the following order:
            | collaborator |
            | Carol King   |
            | Brian Murphy |
            | TestUser1    |
            | TestUser3    |
            | TestUser2    |

    @issue-7459
    Scenario: Progress indicator should not be visible after unselecting the password protection checkbox while sharing through public link
        Given user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user opens the public links dialog of "textfile0.txt" using the client-UI
        And the user toggles the password protection using the client-UI
        And the user toggles the password protection using the client-UI
        Then the password progress indicator should not be visible in the client-UI - expected to fail


    Scenario: Collaborator should not see to whom a file/folder is shared.
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has created folder "Folder" on the server
        And user "Alice" has shared file "/textfile0.txt" on the server with user "Brian" with "read, share" permission
        And user "Alice" has shared folder "Folder" on the server with user "Brian" with "read, share" permission
        And user "Brian" has set up a client with default settings
        When the user opens the sharing dialog of "textfile0.txt" using the client-UI
        Then the error text "The item is not shared with any users or groups" should be displayed in the sharing dialog
        When the user closes the sharing dialog
        And the user opens the sharing dialog of "Folder" using the client-UI
        Then the error text "The item is not shared with any users or groups" should be displayed in the sharing dialog


    Scenario: share file and folder to a group
        Given group "grp1" has been created on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Brian" has been added to group "grp1" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has set up a client with default settings
        When the user adds group "grp1" as collaborator of resource "textfile0.txt" with permissions "edit,share" using the client-UI
        Then group "grp1" should be listed in the collaborators list for file "textfile0.txt" with permissions "edit,share" on the client-UI
        When the user adds group "grp1" as collaborator of resource "simple-folder" with permissions "edit,share" using the client-UI
        Then group "grp1" should be listed in the collaborators list for file "simple-folder" with permissions "edit,share" on the client-UI
        And as "Brian" folder "simple-folder" should exist on the server
        And as "Brian" file "textfile0.txt" should exist on the server


    Scenario: User (non-author) can not share to a group to which the file/folder is already shared
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And group "grp1" has been created on the server
        And user "Brian" on the server has been added to group "grp1"
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has created folder "Folder" on the server
        And user "Alice" has shared file "/textfile0.txt" on the server with user "Brian" with "read, share, update" permission
        And user "Alice" has shared folder "Folder" on the server with user "Brian" with "read, share, update" permission
        And user "Alice" has shared file "/textfile0.txt" on the server with group "grp1" with "read, share, update" permission
        And user "Alice" has shared folder "Folder" on the server with group "grp1" with "read, share, update" permission
        And user "Brian" has set up a client with default settings
        When the user tires to share resource "textfile0.txt" with the group "grp1" using the client-UI
        Then the error "Path already shared with this group" should be displayed
        When the user closes the sharing dialog
        And the user tires to share resource "Folder" with the group "grp1" using the client-UI
        Then the error "Path already shared with this group" should be displayed


    Scenario: sharee edits content of files shared by sharer
        Given user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has uploaded file with content "file inside a folder" to "simple-folder/textfile.txt" on the server
        And user "Alice" has uploaded file with content "file in the root" to "textfile.txt" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared folder "simple-folder" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "all" permissions
        And user "Brian" has set up a client with default settings
        When the user overwrites the file "textfile.txt" with content "overwrite file in the root"
        And the user overwrites the file "simple-folder/textfile.txt" with content "overwrite file inside a folder"
        Then as "Brian" the file "simple-folder/textfile.txt" on the server should have the content "overwrite file inside a folder"
        And as "Brian" the file "textfile.txt" on the server should have the content "overwrite file in the root"
        And as "Alice" the file "simple-folder/textfile.txt" on the server should have the content "overwrite file inside a folder"
        And as "Alice" the file "textfile.txt" on the server should have the content "overwrite file in the root"


    Scenario: sharee tries to edit content of files shared without write permission
        Given user "Alice" has created folder "Parent" on the server
        And user "Alice" has uploaded file with content "file inside a folder" to "Parent/textfile.txt" on the server
        And user "Alice" has uploaded file with content "file in the root" to "textfile.txt" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared folder "Parent" on the server with user "Brian" with "read" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "read" permissions
        And user "Brian" has set up a client with default settings
        When the user tries to overwrite the file "Parent/textfile.txt" with content "overwrite file inside a folder"
        And the user tries to overwrite the file "textfile.txt" with content "overwrite file in the root"
        Then as "Brian" the file "Parent/textfile.txt" on the server should have the content "file inside a folder"
        And as "Brian" the file "textfile.txt" on the server should have the content "file in the root"
        And as "Alice" the file "Parent/textfile.txt" on the server should have the content "file inside a folder"
        And as "Alice" the file "textfile.txt" on the server should have the content "file in the root"


    Scenario: sharee edits shared files and again try to edit after write permission is revoked
        Given user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has created folder "FOLDER" on the server
        And user "Alice" has uploaded file with content "some content" to "FOLDER/simple.txt" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared folder "FOLDER" on the server with user "Brian" with "all" permissions
        And user "Brian" has set up a client with default settings
        When the user overwrites the file "textfile.txt" with content "overwrite ownCloud test text file"
        And the user overwrites the file "FOLDER/simple.txt" with content "overwrite some content"
        Then as "Brian" the file "textfile.txt" on the server should have the content "overwrite ownCloud test text file"
        And as "Alice" the file "textfile.txt" on the server should have the content "overwrite ownCloud test text file"
        And as "Brian" the file "FOLDER/simple.txt" on the server should have the content "overwrite some content"
        And as "Alice" the file "FOLDER/simple.txt" on the server should have the content "overwrite some content"
        When the user adds another account with
            | server   | %local_server% |
            | user     | Alice          |
            | password | 1234           |
        And the user removes permissions "edit" for user "Brian Murphy" of resource "textfile.txt" using the client-UI
        And the user closes the sharing dialog
        And the user removes permissions "edit" for user "Brian Murphy" of resource "FOLDER" using the client-UI
        And user "Brian" tries to overwrite the file "textfile.txt" with content "again overwrite ownCloud test text file"
        And user "Brian" tries to overwrite the file "FOLDER/simple.txt" with content "again overwrite some content"
        Then as "Brian" the file "textfile.txt" on the server should have the content "overwrite ownCloud test text file"
        And as "Brian" the file "FOLDER/simple.txt" on the server should have the content "overwrite some content"
        And as "Alice" the file "textfile.txt" on the server should have the content "overwrite ownCloud test text file"
        And as "Alice" the file "FOLDER/simple.txt" on the server should have the content "overwrite some content"


    Scenario: sharee creates a file and a folder inside a shared folder
        Given user "Alice" has created folder "Parent" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared folder "Parent" on the server with user "Brian" with "all" permissions
        And user "Brian" has set up a client with default settings
        When the user waits for folder "Parent" to be synced
        And user "Brian" creates a file "Parent/localFile.txt" with the following content inside the sync folder
            """
            test content
            """
        And user "Brian" creates a folder "Parent/localFolder" inside the sync folder
        And the user waits for file "Parent/localFile.txt" to be synced
        And the user waits for folder "Parent/localFolder" to be synced
        Then as "Brian" file "Parent/localFile.txt" should exist on the server
        And as "Brian" folder "Parent/localFolder" should exist on the server
        And as "Alice" file "Parent/localFile.txt" should exist on the server
        And as "Alice" folder "Parent/localFolder" should exist on the server


    Scenario: reshare a file/folder
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Carol" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "FOLDER" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has shared file "FOLDER" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "all" permissions
        And user "Brian" has set up a client with default settings
        When the user adds "Carol King" as collaborator of resource "FOLDER" with permissions "edit,share" using the client-UI
        And the user adds "Carol King" as collaborator of resource "textfile.txt" with permissions "edit,share" using the client-UI
        Then user "Carol King" should be listed in the collaborators list for file "FOLDER" with permissions "edit,share" on the client-UI
        And user "Carol King" should be listed in the collaborators list for file "textfile.txt" with permissions "edit,share" on the client-UI


    Scenario: try to reshare a file/folder shared without share permission
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Carol" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "FOLDER" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Alice" has shared file "FOLDER" on the server with user "Brian" with "read" permissions
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "read" permissions
        And user "Brian" has set up a client with default settings
        When the user opens the sharing dialog of "FOLDER" using the client-UI
        Then the error text "The file can not be shared because it was shared without sharing permission." should be displayed in the sharing dialog
        When the user opens the sharing dialog of "textfile.txt" using the client-UI
        Then the error text "The file can not be shared because it was shared without sharing permission." should be displayed in the sharing dialog


    Scenario: unshare a shared file
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "textfile0.txt" on the server
        And user "Alice" has shared file "textfile0.txt" on the server with user "Brian" with "all" permissions
        And user "Alice" has set up a client with default settings
        When the user unshares the resource "textfile0.txt" for collaborator "Brian Murphy" using the client-UI
        Then the text "The item is not shared with any users or groups" should be displayed in the sharing dialog
        And as "Brian" file "textfile0.txt" on the server should not exist


    Scenario: unshare a shared folder
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "textfile0.txt" on the server
        And user "Alice" has shared folder "simple-folder" on the server with user "Brian" with "all" permissions
        And user "Alice" has set up a client with default settings
        When the user unshares the resource "simple-folder" for collaborator "Brian Murphy" using the client-UI
        Then the text "The item is not shared with any users or groups" should be displayed in the sharing dialog
        And as "Brian" folder "simple-folder" on the server should not exist


    Scenario: share a file with many users
        Given user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Carol" has been created on the server with default attributes and without skeleton files
        And user "David" has been created on the server with default attributes and without skeleton files
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user adds following collaborators of resource "textfile0.txt" using the client-UI
            | user         | permissions |
            | Brian Murphy | edit,share  |
            | Carol King   | edit,share  |
            | David Lopez  | edit,share  |
        Then the following users should be listed in as collaborators for file "textfile0.txt" on the client-UI
            | user         | permissions |
            | Brian Murphy | edit,share  |
            | Carol King   | edit,share  |
            | David Lopez  | edit,share  |


    @issue-7423
    Scenario: unshare a reshared file
        Given user "Alice" has uploaded file with content "ownCloud test text file" to "textfile.txt" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Carol" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared folder "textfile.txt" on the server with user "Brian"
        And user "Brian" has shared folder "textfile.txt" on the server with user "Carol"
        And user "Brian" has set up a client with default settings
        When the user unshares the resource "textfile.txt" for collaborator "Carol King" using the client-UI
        Then the text "The item is not shared with any users or groups" should be displayed in the sharing dialog

    @smokeTest
    Scenario: simple sharing of a file by public link without password
        Given user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user creates a new public link for file "textfile0.txt" without password using the client-UI
        Then as user "Alice" the file "textfile0.txt" should have a public link on the server
        And the public should be able to download the file "textfile0.txt" without password from the last created public link by "Alice" on the server


    Scenario: sharing of a file by public link and deleting the link
        Given user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has set up a client with default settings
        And user "Alice" has created a public link on the server with following settings
            | path     | textfile0.txt |
            | name     | Public-link   |
        When the user deletes the public link for file "textfile0.txt"
        Then as user "Alice" the file "/textfile0.txt" should not have any public link on the server


    Scenario Outline: simple sharing of a file by public link with password
        Given user "Alice" has set up a client with default settings
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        When the user creates a new public link for file "textfile0.txt" with password "<password>" using the client-UI
        Then as user "Alice" the file "textfile0.txt" should have a public link on the server
        And the public should be able to download the file "textfile0.txt" with password "<password>" from the last created public link by "Alice" on the server
        Examples:
            | password     |
            | password1234 |
            | p@$s!23      |


    Scenario: sharing of a file by public link with password and changing the password
        Given user "Alice" has set up a client with default settings
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has created a public link on the server with following settings
            | path     | textfile0.txt |
            | name     | Public-link   |
            | password | 1234          |
        When the user opens the public links dialog of "textfile0.txt" using the client-UI
        And the user changes the password of public link "Public-link" to "password1234" using the client-UI
        Then as user "Alice" the file "textfile0.txt" should have a public link on the server
        And the public should be able to download the file "textfile0.txt" with password "password1234" from the last created public link by "Alice" on the server



    Scenario: simple sharing of a file by public link with password and expiration date
        Given user "Alice" has set up a client with default settings
        And user "Alice" has uploaded file with content "ownCloud test text file" to "/textfile.txt" on the server
        When the user creates a new public link with following settings using the client-UI:
            | path       | textfile.txt |
            | password   | pass123      |
            | expireDate | 2031-10-14   |
        Then as user "Alice" the file "textfile.txt" should have a public link on the server
        And the fields of the last public link share response of user "Alice" should include on the server
            | expireDate | 2031-10-14 |
        And the public should be able to download the file "textfile.txt" with password "pass123" from the last created public link by "Alice" on the server


    @skip @issue-9321
    Scenario: user changes the expiration date of an already existing public link using webUI
        Given user "Alice" has uploaded file with content "ownCloud test text file 0" to "/textfile0.txt" on the server
        And user "Alice" has set up a client with default settings
        And user "Alice" has created a public link on the server with following settings
            | path       | textfile0.txt |
            | name       | Public link   |
            | expireDate | 2031-10-14    |
        When the user opens the public links dialog of "textfile0.txt" using the client-UI
        And the user edits the public link named "Public link" of file "textfile0.txt" changing following
            | expireDate | 2038-07-21 |
        Then the fields of the last public link share response of user "Alice" should include on the server
            | expireDate | 2038-07-21 |

    @smokeTest
    Scenario: simple sharing of a folder by public link without password
        Given user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has set up a client with default settings
        When the user creates a new public link with permissions "Download / View" for folder "simple-folder" without password using the client-UI
        Then as user "Alice" the folder "simple-folder" should have a public link on the server
        And the public should be able to download the folder "lorem.txt" without password from the last created public link by "Alice" on the server


    Scenario: simple sharing of a folder by public link with password
        Given user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has set up a client with default settings
        When the user creates a new public link with permissions "Download / View" for folder "simple-folder" with password "pass123" using the client-UI
        Then as user "Alice" the folder "simple-folder" should have a public link on the server
        And the public should be able to download the folder "lorem.txt" with password "pass123" from the last created public link by "Alice" on the server

    @skip @issue-9321
    Scenario: user changes the expiration date of an already existing public link for folder using client-UI
        Given user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has set up a client with default settings
        And user "Alice" has created a public link on the server with following settings
            | path        | simple-folder                |
            | name        | Public link                  |
            | expireDate  | 2031-10-14                   |
            | permissions | read, update, create, delete |
        When the user opens the public links dialog of "simple-folder" using the client-UI
        And the user edits the public link named "Public link" of file "simple-folder" changing following
            | expireDate | 2038-07-21 |
        Then the fields of the last public link share response of user "Alice" on the server should include
            | expireDate | 2038-07-21 |


    Scenario Outline: simple sharing of folder by public link with different roles
        Given user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has set up a client with default settings
        When the user creates a new public link for folder "simple-folder" using the client-UI with these details:
            | role | <role> |
        Then user "Alice" on the server should have a share with these details:
            | field       | value          |
            | share_type  | public_link    |
            | uid_owner   | Alice          |
            | permissions | <permissions>  |
            | path        | /simple-folder |
            | name        | Public link    |
        Examples:
            | role        | permissions                  |
            | Viewer      | read                         |
            | Editor      | read, update, create, delete |
            | Contributor | create                       |


    Scenario: sharing by public link with "Uploader" role
        Given user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has created file "simple-folder/lorem.txt" on the server
        And user "Alice" has set up a client with default settings
        When the user creates a new public link for folder "simple-folder" using the client-UI with these details:
            | role | Contributor |
        Then user "Alice" on the server should have a share with these details:
            | field       | value          |
            | share_type  | public_link    |
            | uid_owner   | Alice          |
            | permissions | create         |
            | path        | /simple-folder |
            | name        | Public link    |
        And the public should not be able to download the file "lorem.txt" from the last created public link by "Alice" on the server


    Scenario Outline: change collaborator permissions of a file & folder
        Given the setting "shareapi_auto_accept_share" on the server of app "core" has been set to "yes"
        And the administrator on the server has set the default folder for received shares to "Shares"
        And user "Alice" has created folder "simple-folder" on the server
        And user "Alice" has created file "lorem.txt" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared folder "simple-folder" on the server with user "Brian" with "all" permissions
        And user "Alice" has shared file "lorem.txt" on the server with user "Brian" with "all" permissions
        And user "Alice" has set up a client with default settings
        When the user removes permissions "<permissions>" for user "Brian Murphy" of resource "simple-folder" using the client-UI
        Then "<permissions>" permissions should not be displayed for user "Brian Murphy" for resource "simple-folder" on the client-UI
        When the user closes the sharing dialog
        And the user removes permissions "<permissions>" for user "Brian Murphy" of resource "lorem.txt" using the client-UI
        Then "<permissions>" permissions should not be displayed for user "Brian Murphy" for resource "lorem.txt" on the client-UI
        And user "Alice" on the server should have a share with these details:
            | field       | value                        |
            | uid_owner   | Alice                        |
            | share_with  | Brian                        |
            | share_type  | user                         |
            | file_target | /Shares/simple-folder        |
            | item_type   | folder                       |
            | permissions | <expected-folder-permission> |
        And user "Alice" on the server should have a share with these details:
            | field       | value                      |
            | uid_owner   | Alice                      |
            | share_with  | Brian                      |
            | share_type  | user                       |
            | file_target | /Shares/lorem.txt          |
            | item_type   | file                       |
            | permissions | <expected-file-permission> |
        Examples:
            | permissions | expected-folder-permission   | expected-file-permission |
            | edit        | read, share                  | read, share              |
            | share       | read, update, create, delete | read,update              |
            | edit,share  | read                         | read                     |


    Scenario: sharee deletes a file shared by sharer
        Given the setting "shareapi_auto_accept_share" on the server of app "core" has been set to "yes"
        And the administrator on the server has set the default folder for received shares to "Shares"
        And user "Alice" has uploaded file with content "ownCloud test text file 0" to "textfile.txt" on the server
        And user "Brian" has been created on the server with default attributes and without skeleton files
        And user "Alice" has shared file "textfile.txt" on the server with user "Brian" with "all" permissions
        And user "Brian" has set up a client with default settings
        When the user waits for file "Shares/textfile.txt" to be synced
        And the user deletes the file "Shares/textfile.txt"
        And the user waits for the files to sync
        Then as "Brian" file "Shares/textfile0.txt" on the server should not exist
        And as "Alice" file "textfile0.txt" on the server should not exist
