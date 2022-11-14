/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "./windows.h"
#ifdef _WINDOWS_
   #pragma region Define aliases via auto&

      #ifdef COBB_WIN32_IN_NAMESPACE
      namespace win32 {
      #endif

      #pragma push_macro("MAKE_WIN32_ALIAS")
      #pragma push_macro("MAKE_WIN32_ALIAS_A_ONLY")
      #if UNICODE
         #define MAKE_WIN32_ALIAS(Name) extern decltype(Name##W)& Name = (Name##W);
         #define MAKE_WIN32_ALIAS_A_ONLY(Name) ;
      #else
         #define MAKE_WIN32_ALIAS(Name) extern decltype(Name##A)& Name = (Name##A);
         #define MAKE_WIN32_ALIAS_A_ONLY(Name) extern decltype(Name##A)& Name = (Name##A);
      #endif

      //
      // We need to actually supply values for all these references we've created. 
      // This is a little trickier, because some Windows headers only conditionally 
      // define API functions, and so we need to dig into them and check those same 
      // conditions.
      //
         
      #pragma region Commdlg.h
         MAKE_WIN32_ALIAS(ChooseColor)
         MAKE_WIN32_ALIAS(ChooseFont)
         MAKE_WIN32_ALIAS(FindText)
         MAKE_WIN32_ALIAS(GetFileTitle)
         MAKE_WIN32_ALIAS(GetOpenFileName)
         MAKE_WIN32_ALIAS(GetSaveFileName)
         MAKE_WIN32_ALIAS(PageSetupDlg)
         MAKE_WIN32_ALIAS(PrintDlg)
         MAKE_WIN32_ALIAS(PrintDlgEx)
         MAKE_WIN32_ALIAS(ReplaceText)
      #pragma endregion
      #pragma region Datetimeapi.h
         MAKE_WIN32_ALIAS(GetDateFormat)
         MAKE_WIN32_ALIAS(GetTimeFormat)
      #pragma endregion
      #ifdef COBB_WIN32_INCLUDE_ENCLAVE
         #pragma region Enclaveapi.h
            MAKE_WIN32_ALIAS(LoadEnclaveImage)
         #pragma endregion
      #endif
      #ifdef COBB_WIN32_INCLUDE_IMM
         #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            #pragma region Imm.h
               MAKE_WIN32_ALIAS(ImmConfigureIME)
               MAKE_WIN32_ALIAS(ImmEnumRegisterWord)
               MAKE_WIN32_ALIAS(ImmEscape)
               MAKE_WIN32_ALIAS(ImmGetCandidateList)
               MAKE_WIN32_ALIAS(ImmGetCandidateListCount)
               MAKE_WIN32_ALIAS(ImmGetCompositionFont)
               MAKE_WIN32_ALIAS(ImmGetCompositionString)
               MAKE_WIN32_ALIAS(ImmGetConversionList)
               MAKE_WIN32_ALIAS(ImmGetDescription)
               MAKE_WIN32_ALIAS(ImmGetGuideLine)
               MAKE_WIN32_ALIAS(ImmGetIMEFileName)
               MAKE_WIN32_ALIAS(ImmGetImeMenuItems)
               MAKE_WIN32_ALIAS(ImmGetRegisterWordStyle)
               MAKE_WIN32_ALIAS(ImmInstallIME)
               MAKE_WIN32_ALIAS(ImmIsUIMessage)
               MAKE_WIN32_ALIAS(ImmRegisterWord)
               MAKE_WIN32_ALIAS(ImmSetCompositionFont)
               MAKE_WIN32_ALIAS(ImmSetCompositionString)
               MAKE_WIN32_ALIAS(ImmUnregisterWord)
            #pragma endregion
            #pragma region Immdev.h (excluding Imm.h)
               MAKE_WIN32_ALIAS_A_ONLY(ImmRequestMessage)
            #pragma endregion
         #endif
      #endif
      #pragma region Libloaderapi.h
         #pragma region System Services
            MAKE_WIN32_ALIAS(GetModuleFileName)
            MAKE_WIN32_ALIAS(GetModuleHandle)
            MAKE_WIN32_ALIAS(GetModuleHandleEx)
            MAKE_WIN32_ALIAS(LoadLibrary)
            MAKE_WIN32_ALIAS(LoadLibraryEx)
         #pragma endregion
         MAKE_WIN32_ALIAS(EnumResourceLanguagesEx)
         MAKE_WIN32_ALIAS(EnumResourceNamesEx)
         MAKE_WIN32_ALIAS(EnumResourceTypesEx)
         MAKE_WIN32_ALIAS(FindResourceEx)
         MAKE_WIN32_ALIAS(FindResource)
      #pragma endregion
      #ifdef COBB_WIN32_INCLUDE_COMMON_CONTROLS
         #pragma region Prsht.h
            MAKE_WIN32_ALIAS(CreatePropertySheetPage)
            MAKE_WIN32_ALIAS(PropertySheet)
         #pragma endregion
      #endif
      #ifdef COBB_WIN32_INCLUDE_SHELL_API
         #pragma region Shellapi.h
            MAKE_WIN32_ALIAS(CommandLineToArgv)
            MAKE_WIN32_ALIAS(DoEnvironmentSubst)
            MAKE_WIN32_ALIAS(DragQueryFile)
            MAKE_WIN32_ALIAS(ExtractAssociatedIcon)
            MAKE_WIN32_ALIAS(ExtractAssociatedIconEx)
            MAKE_WIN32_ALIAS(ExtractIcon)
            MAKE_WIN32_ALIAS(ExtractIconEx)
            MAKE_WIN32_ALIAS(FindExecutable)
            MAKE_WIN32_ALIAS(SHCreateProcessAsUser)
            MAKE_WIN32_ALIAS(SHEmptyRecycleBin)
            MAKE_WIN32_ALIAS(SHEnumerateUnreadMailAccounts)
            MAKE_WIN32_ALIAS(SHFileOperation)
            MAKE_WIN32_ALIAS(SHGetDiskFreeSpaceEx)
            MAKE_WIN32_ALIAS(SHGetFileInfo)
            MAKE_WIN32_ALIAS(SHGetNewLinkInfo)
            MAKE_WIN32_ALIAS(SHGetUnreadMailCount)
            MAKE_WIN32_ALIAS(SHInvokePrinterCommand)
            MAKE_WIN32_ALIAS(SHQueryRecycleBin)
            MAKE_WIN32_ALIAS(SHSetUnreadMailCount)
            MAKE_WIN32_ALIAS(ShellAbout)
            MAKE_WIN32_ALIAS(ShellExecute)
            MAKE_WIN32_ALIAS(ShellExecuteEx)
            MAKE_WIN32_ALIAS(ShellMessageBox)
            MAKE_WIN32_ALIAS(Shell_NotifyIcon)
         #pragma endregion
      #endif
      #pragma region Synchapi.h
         MAKE_WIN32_ALIAS(CreateEvent)
         MAKE_WIN32_ALIAS(CreateEventEx)
         MAKE_WIN32_ALIAS(CreateMutex)
         MAKE_WIN32_ALIAS(CreateMutexEx)
         MAKE_WIN32_ALIAS(CreateSemaphore)
         MAKE_WIN32_ALIAS(CreateSemaphoreEx)
         MAKE_WIN32_ALIAS(CreateWaitableTimer)
         MAKE_WIN32_ALIAS(CreateWaitableTimerEx)
         MAKE_WIN32_ALIAS(OpenEvent)
         MAKE_WIN32_ALIAS(OpenMutex)
         MAKE_WIN32_ALIAS(OpenSemaphore)
         MAKE_WIN32_ALIAS(OpenWaitableTimer)
         MAKE_WIN32_ALIAS(SleepConditionVariableSR)
      #pragma endregion
      #pragma region Winbase.h (excluding Libloaderapi.h and Synchapi.h)
         #pragma region Security and Identity
            #ifdef COBB_WIN32_INCLUDE_ADVANCED_API
               MAKE_WIN32_ALIAS(AccessCheckAndAuditAlarm)
               MAKE_WIN32_ALIAS(AccessCheckByTypeAndAuditAlarm)
               MAKE_WIN32_ALIAS(AccessCheckByTypeResultListAndAuditAlarm)
               MAKE_WIN32_ALIAS(AccessCheckByTypeResultListAndAuditAlarmByHandle)
               MAKE_WIN32_ALIAS(GetFileSecurity)
               MAKE_WIN32_ALIAS(LogonUser)
               MAKE_WIN32_ALIAS(LogonUserEx)
               MAKE_WIN32_ALIAS(LookupAccountName)
               MAKE_WIN32_ALIAS(LookupAccountSid)
               MAKE_WIN32_ALIAS(LookupAccountSidLocal)
               MAKE_WIN32_ALIAS(LookupPrivilegeDisplayName)
               MAKE_WIN32_ALIAS(LookupPrivilegeName)
               MAKE_WIN32_ALIAS(LookupPrivilegeValue)
               MAKE_WIN32_ALIAS(ObjectCloseAuditAlarm)
               MAKE_WIN32_ALIAS(ObjectDeleteAuditAlarm)
               MAKE_WIN32_ALIAS(ObjectOpenAuditAlarm)
               MAKE_WIN32_ALIAS(ObjectPrivilegeAuditAlarm)
               MAKE_WIN32_ALIAS(PrivilegedServiceAuditAlarm)
               MAKE_WIN32_ALIAS(SetFileSecurity)
            #endif
         #pragma endregion
         MAKE_WIN32_ALIAS(AddAtom)
         MAKE_WIN32_ALIAS(BackupEventLog)
         MAKE_WIN32_ALIAS(BeginUpdateResource)
         MAKE_WIN32_ALIAS(BuildCommDCB)
         MAKE_WIN32_ALIAS(BuildCommDCBAndTimeouts)
         MAKE_WIN32_ALIAS(CallNamedPipe)
         MAKE_WIN32_ALIAS(CheckNameLegalDOS8Dot3)
         MAKE_WIN32_ALIAS(ClearEventLog)
         MAKE_WIN32_ALIAS(CommConfigDialog)
         MAKE_WIN32_ALIAS(CopyFile)
         MAKE_WIN32_ALIAS(CopyFileEx)
         MAKE_WIN32_ALIAS(CopyFileTransacted)
         MAKE_WIN32_ALIAS(CreateActCtx)
         MAKE_WIN32_ALIAS(CreateBoundaryDescriptor)
         MAKE_WIN32_ALIAS(CreateDirectoryEx)
         MAKE_WIN32_ALIAS(CreateDirectoryTransacted)
         MAKE_WIN32_ALIAS(CreateFileMapping)
         MAKE_WIN32_ALIAS(CreateFileMappingNuma)
         MAKE_WIN32_ALIAS(CreateFileTransacted)
         MAKE_WIN32_ALIAS(CreateHardLink)
         MAKE_WIN32_ALIAS(CreateHardLinkTransacted)
         MAKE_WIN32_ALIAS(CreateJobObject)
         MAKE_WIN32_ALIAS(CreateMailslot)
         MAKE_WIN32_ALIAS(CreateNamedPipe)
         MAKE_WIN32_ALIAS(CreatePrivateNamespace)
         MAKE_WIN32_ALIAS(CreateProcessWithLogon)
         MAKE_WIN32_ALIAS(CreateProcessWithToken)
         MAKE_WIN32_ALIAS(CreateSymbolicLink)
         MAKE_WIN32_ALIAS(CreateSymbolicLinkTransacted)
         MAKE_WIN32_ALIAS(DecryptFile)
         MAKE_WIN32_ALIAS(DefineDosDevice)
         MAKE_WIN32_ALIAS(DeleteFileTransacted)
         MAKE_WIN32_ALIAS(DeleteVolumeMountPoint)
         MAKE_WIN32_ALIAS(DnsHostnameToComputerName)
         MAKE_WIN32_ALIAS(EncryptFile)
         MAKE_WIN32_ALIAS(EndUpdateResource)
         MAKE_WIN32_ALIAS(EnumResourceLanguages)
         MAKE_WIN32_ALIAS(EnumResourceTypes)
         MAKE_WIN32_ALIAS(FileEncryptionStatus)
         MAKE_WIN32_ALIAS(FindActCtxSectionString)
         MAKE_WIN32_ALIAS(FindAtom)
         MAKE_WIN32_ALIAS(FindFirstFileNameTransacted)
         MAKE_WIN32_ALIAS(FindFirstFileTransacted)
         MAKE_WIN32_ALIAS(FindFirstStreamTransacted)
         MAKE_WIN32_ALIAS(FindFirstVolume)
         MAKE_WIN32_ALIAS(FindFirstVolumeMountPoint)
         MAKE_WIN32_ALIAS(FindNextVolume)
         MAKE_WIN32_ALIAS(FindNextVolumeMountPoint)
         MAKE_WIN32_ALIAS(FormatMessage)
         MAKE_WIN32_ALIAS(GetAtomName)
         MAKE_WIN32_ALIAS(GetBinaryType)
         MAKE_WIN32_ALIAS(GetCompressedFileSizeTransacted)
         MAKE_WIN32_ALIAS(GetComputerName)
         MAKE_WIN32_ALIAS(GetCurrentHwProfile)
         MAKE_WIN32_ALIAS(GetDefaultCommConfig)
         MAKE_WIN32_ALIAS(GetDllDirectory)
         MAKE_WIN32_ALIAS(GetFileAttributesTransacted)
         MAKE_WIN32_ALIAS(GetFirmwareEnvironmentVariable)
         MAKE_WIN32_ALIAS(GetFirmwareEnvironmentVariableEx)
         MAKE_WIN32_ALIAS(GetFullPathNameTransacted)
         MAKE_WIN32_ALIAS(GetLogicalDriveStrings)
         MAKE_WIN32_ALIAS(GetLongPathNameTransacted)
         MAKE_WIN32_ALIAS(GetNamedPipeClientComputerName)
         MAKE_WIN32_ALIAS(GetNamedPipeHandleState)
         MAKE_WIN32_ALIAS(GetPrivateProfileInt)
         MAKE_WIN32_ALIAS(GetPrivateProfileSection)
         MAKE_WIN32_ALIAS(GetPrivateProfileSectionNames)
         MAKE_WIN32_ALIAS(GetPrivateProfileString)
         MAKE_WIN32_ALIAS(GetPrivateProfileStruct)
         MAKE_WIN32_ALIAS(GetProfileInt)
         MAKE_WIN32_ALIAS(GetProfileSection)
         MAKE_WIN32_ALIAS(GetProfileString)
         MAKE_WIN32_ALIAS(GetShortPathName)
         MAKE_WIN32_ALIAS(GetUserName)
         MAKE_WIN32_ALIAS(GetVolumeNameForVolumeMountPoint)
         MAKE_WIN32_ALIAS(GetVolumePathName)
         MAKE_WIN32_ALIAS(GetVolumePathNamesForVolumeName)
         MAKE_WIN32_ALIAS(GlobalAddAtom)
         MAKE_WIN32_ALIAS(GlobalAddAtomEx)
         MAKE_WIN32_ALIAS(GlobalFindAtom)
         MAKE_WIN32_ALIAS(GlobalGetAtomName)
         MAKE_WIN32_ALIAS(IsBadStringPtr)
         MAKE_WIN32_ALIAS(MoveFile)
         MAKE_WIN32_ALIAS(MoveFileEx)
         MAKE_WIN32_ALIAS(MoveFileTransacted)
         MAKE_WIN32_ALIAS(MoveFileWithProgress)
         MAKE_WIN32_ALIAS(OpenBackupEventLog)
         MAKE_WIN32_ALIAS(OpenEncryptedFileRaw)
         MAKE_WIN32_ALIAS(OpenEventLog)
         MAKE_WIN32_ALIAS(OpenFileMapping)
         MAKE_WIN32_ALIAS(OpenJobObject)
         MAKE_WIN32_ALIAS(OpenPrivateNamespace)
         MAKE_WIN32_ALIAS(QueryActCtxSettings)
         MAKE_WIN32_ALIAS(QueryActCtx)
         MAKE_WIN32_ALIAS(QueryDosDevice)
         MAKE_WIN32_ALIAS(QueryFullProcessImageName)
         MAKE_WIN32_ALIAS(ReadDirectoryChangesEx)
         MAKE_WIN32_ALIAS(ReadDirectoryChanges)
         MAKE_WIN32_ALIAS(ReadEventLog)
         MAKE_WIN32_ALIAS(RegisterEventSource)
         MAKE_WIN32_ALIAS(RemoveDirectoryTransacted)
         MAKE_WIN32_ALIAS(ReplaceFile)
         MAKE_WIN32_ALIAS(ReportEvent)
         MAKE_WIN32_ALIAS(SetDefaultCommConfig)
         MAKE_WIN32_ALIAS(SetDllDirectory)
         MAKE_WIN32_ALIAS(SetFileAttributesTransacted)
         MAKE_WIN32_ALIAS(SetFileShortName)
         MAKE_WIN32_ALIAS(SetFirmwareEnvironmentVariable)
         MAKE_WIN32_ALIAS(SetFirmwareEnvironmentVariableEx)
         MAKE_WIN32_ALIAS(SetVolumeLabel)
         MAKE_WIN32_ALIAS(SetVolumeMountPoint)
         MAKE_WIN32_ALIAS(UpdateResource)
         MAKE_WIN32_ALIAS(VerifyVersionInfo)
         MAKE_WIN32_ALIAS(WaitNamedPipe)
         MAKE_WIN32_ALIAS(WritePrivateProfileSection)
         MAKE_WIN32_ALIAS(WritePrivateProfileString)
         MAKE_WIN32_ALIAS(WritePrivateProfileStruct)
         MAKE_WIN32_ALIAS(WriteProfileSection)
         MAKE_WIN32_ALIAS(WriteProfileString)
      #pragma endregion
      #ifdef COBB_WIN32_INCLUDE_CRYPT
         #pragma region Wincrypt.h
            MAKE_WIN32_ALIAS(CertAddEncodedCertificateToSystemStore)
            MAKE_WIN32_ALIAS(CertGetNameString)
            MAKE_WIN32_ALIAS(CertNameToStr)
            MAKE_WIN32_ALIAS(CertOpenSystemStore)
            MAKE_WIN32_ALIAS(CertRDNValueToStr)
            MAKE_WIN32_ALIAS(CertStrToName)
            MAKE_WIN32_ALIAS(CryptAcquireContext)
            MAKE_WIN32_ALIAS(CryptBinaryToString)
            MAKE_WIN32_ALIAS(CryptEnumProviderTypes)
            MAKE_WIN32_ALIAS(CryptEnumProviders)
            MAKE_WIN32_ALIAS(CryptGetDefaultProvider)
            MAKE_WIN32_ALIAS(CryptRetrieveObjectByUrl)
            MAKE_WIN32_ALIAS(CryptSetProvider)
            MAKE_WIN32_ALIAS(CryptSetProviderEx)
            MAKE_WIN32_ALIAS(CryptSignHash)
            MAKE_WIN32_ALIAS(CryptStringToBinary)
            MAKE_WIN32_ALIAS(CryptVerifySignature)
         #pragma endregion
      #endif
      #pragma region Wingdi.h
         MAKE_WIN32_ALIAS(AddFontResource)
         MAKE_WIN32_ALIAS(AddFontResourceEx)
         MAKE_WIN32_ALIAS(CopyEnhMetaFile)
         MAKE_WIN32_ALIAS(CopyMetaFile)
         MAKE_WIN32_ALIAS(CreateDC)
         MAKE_WIN32_ALIAS(CreateEnhMetaFile)
         MAKE_WIN32_ALIAS(CreateFont)
         MAKE_WIN32_ALIAS(CreateFontIndirect)
         MAKE_WIN32_ALIAS(CreateFontIndirectEx)
         MAKE_WIN32_ALIAS(CreateIC)
         MAKE_WIN32_ALIAS(CreateMetaFile)
         MAKE_WIN32_ALIAS(CreateScalableFontResource)
         MAKE_WIN32_ALIAS(EnumFontFamilies)
         MAKE_WIN32_ALIAS(EnumFontFamiliesEx)
         MAKE_WIN32_ALIAS(EnumFonts)
         MAKE_WIN32_ALIAS(ExtTextOut)
         MAKE_WIN32_ALIAS(GetCharABCWidths)
         MAKE_WIN32_ALIAS(GetCharABCWidthsFloat)
         MAKE_WIN32_ALIAS(GetCharWidth)
         MAKE_WIN32_ALIAS(GetCharWidth32)
         MAKE_WIN32_ALIAS(GetCharWidthFloat)
         MAKE_WIN32_ALIAS(GetCharacterPlacement)
         MAKE_WIN32_ALIAS(GetEnhMetaFile)
         MAKE_WIN32_ALIAS(GetEnhMetaFileDescription)
         MAKE_WIN32_ALIAS(GetGlyphIndices)
         MAKE_WIN32_ALIAS(GetGlyphOutline)
         MAKE_WIN32_ALIAS(GetKerningPairs)
         MAKE_WIN32_ALIAS(GetMetaFile)
         MAKE_WIN32_ALIAS(GetObject)
         MAKE_WIN32_ALIAS(GetOutlineTextMetrics)
         MAKE_WIN32_ALIAS(GetTextExtentExPoint)
         MAKE_WIN32_ALIAS(GetTextExtentPoint)
         MAKE_WIN32_ALIAS(GetTextExtentPoint32)
         MAKE_WIN32_ALIAS(GetTextFace)
         MAKE_WIN32_ALIAS(GetTextMetrics)
         MAKE_WIN32_ALIAS_A_ONLY(LineDD)
         MAKE_WIN32_ALIAS(PolyTextOut)
         MAKE_WIN32_ALIAS(RemoveFontResource)
         MAKE_WIN32_ALIAS(RemoveFontResourceEx)
         MAKE_WIN32_ALIAS(ResetDC)
         MAKE_WIN32_ALIAS(TextOut)
      #pragma endregion
      #pragma region Winreg.h
         #pragma region System Services
            MAKE_WIN32_ALIAS(AbortSystemShutdown)
            MAKE_WIN32_ALIAS(InitiateShutdown)
            MAKE_WIN32_ALIAS(InitiateSystemShutdown)
            MAKE_WIN32_ALIAS(InitiateSystemShutdownEx)
         #pragma endregion
         MAKE_WIN32_ALIAS(RegConnectRegistry)
         MAKE_WIN32_ALIAS(RegCopyTree)
         MAKE_WIN32_ALIAS(RegCreateKey)
         MAKE_WIN32_ALIAS(RegCreateKeyEx)
         MAKE_WIN32_ALIAS(RegCreateKeyTransacted)
         MAKE_WIN32_ALIAS(RegDeleteKey)
         MAKE_WIN32_ALIAS(RegDeleteKeyEx)
         MAKE_WIN32_ALIAS(RegDeleteKeyTransacted)
         MAKE_WIN32_ALIAS(RegDeleteKeyValue)
         MAKE_WIN32_ALIAS(RegDeleteTree)
         MAKE_WIN32_ALIAS(RegDeleteValue)
         MAKE_WIN32_ALIAS(RegEnumKey)
         MAKE_WIN32_ALIAS(RegEnumKeyEx)
         MAKE_WIN32_ALIAS(RegEnumValue)
         MAKE_WIN32_ALIAS(RegGetValue)
         MAKE_WIN32_ALIAS(RegLoadAppKey)
         MAKE_WIN32_ALIAS(RegLoadKey)
         MAKE_WIN32_ALIAS(RegLoadMUIString)
         MAKE_WIN32_ALIAS(RegOpenKey)
         MAKE_WIN32_ALIAS(RegOpenKeyEx)
         MAKE_WIN32_ALIAS(RegOpenKeyTransacted)
         MAKE_WIN32_ALIAS(RegQueryInfoKey)
         MAKE_WIN32_ALIAS(RegQueryMultipleValues)
         MAKE_WIN32_ALIAS(RegQueryValue)
         MAKE_WIN32_ALIAS(RegQueryValueEx)
         MAKE_WIN32_ALIAS(RegReplaceKey)
         MAKE_WIN32_ALIAS(RegRestoreKey)
         MAKE_WIN32_ALIAS(RegSaveKey)
         MAKE_WIN32_ALIAS(RegSaveKeyEx)
         MAKE_WIN32_ALIAS(RegSetKeyValue)
         MAKE_WIN32_ALIAS(RegSetValue)
         MAKE_WIN32_ALIAS(RegSetValueEx)
         MAKE_WIN32_ALIAS(RegUnLoadKey)
      #pragma endregion
      #ifdef COBB_WIN32_INCLUDE_SCARD
         #pragma region Winscard.h
            #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
               MAKE_WIN32_ALIAS(GetOpenCardName)
               MAKE_WIN32_ALIAS(SCardAddReaderToGroup)
               MAKE_WIN32_ALIAS(SCardConnect)
               MAKE_WIN32_ALIAS(SCardForgetCardType)
               MAKE_WIN32_ALIAS(SCardForgetReader)
               MAKE_WIN32_ALIAS(SCardForgetReaderGroup)
               MAKE_WIN32_ALIAS(SCardGetCardTypeProviderName)
               MAKE_WIN32_ALIAS(SCardGetDeviceTypeId)
               MAKE_WIN32_ALIAS(SCardGetProviderId)
               MAKE_WIN32_ALIAS(SCardGetReaderDeviceInstanceId)
               MAKE_WIN32_ALIAS(SCardGetReaderIcon)
               MAKE_WIN32_ALIAS(SCardGetStatusChange)
               MAKE_WIN32_ALIAS(SCardIntroduceCardType)
               MAKE_WIN32_ALIAS(SCardIntroduceReader)
               MAKE_WIN32_ALIAS(SCardIntroduceReaderGroup)
               MAKE_WIN32_ALIAS(SCardListCards)
               MAKE_WIN32_ALIAS(SCardListInterfaces)
               MAKE_WIN32_ALIAS(SCardListReaderGroups)
               MAKE_WIN32_ALIAS(SCardListReaders)
               MAKE_WIN32_ALIAS(SCardListReadersWithDeviceInstanceId)
               MAKE_WIN32_ALIAS(SCardLocateCards)
               MAKE_WIN32_ALIAS(SCardLocateCardsByATR)
               MAKE_WIN32_ALIAS(SCardReadCache)
               MAKE_WIN32_ALIAS(SCardRemoveReaderFromGroup)
               MAKE_WIN32_ALIAS(SCardSetCardTypeProviderName)
               MAKE_WIN32_ALIAS(SCardStatus)
               MAKE_WIN32_ALIAS(SCardUIDlgSelectCard)
               MAKE_WIN32_ALIAS(SCardWriteCache)
            #endif
         #pragma endregion
      #endif
      #pragma region Winsvc.h
         MAKE_WIN32_ALIAS(ChangeServiceConfig2)
         MAKE_WIN32_ALIAS(ControlServiceEx)
         MAKE_WIN32_ALIAS(EnumDependentServices)
         MAKE_WIN32_ALIAS(EnumServicesStatus)
         MAKE_WIN32_ALIAS(EnumServicesStatusEx)
         MAKE_WIN32_ALIAS(GetServiceDisplayName)
         MAKE_WIN32_ALIAS(GetServiceKeyName)
         MAKE_WIN32_ALIAS(NotifyServiceStatusChange)
         MAKE_WIN32_ALIAS(OpenSCManager)
         MAKE_WIN32_ALIAS(OpenService)
         MAKE_WIN32_ALIAS(QueryServiceConfig)
         MAKE_WIN32_ALIAS(QueryServiceConfig2)
         MAKE_WIN32_ALIAS(QueryServiceLockStatus)
         MAKE_WIN32_ALIAS(RegisterServiceCtrlHandler)
         MAKE_WIN32_ALIAS(RegisterServiceCtrlHandlerEx)
         MAKE_WIN32_ALIAS(StartService)
         MAKE_WIN32_ALIAS(StartServiceCtrlDispatcher)
      #pragma endregion
      #pragma region Winuser.h
         //
         // This header is massive enough, and has enough transitive includes, that 
         // Microsoft doesn't simply let you browse everything in it. Microsoft Learn 
         // (formerly MSDN) divides the header up by category.
         //
         #pragma region Data Exchange
            MAKE_WIN32_ALIAS(RegisterClipboardFormat)
         #pragma endregion
         #pragma region Dialog Boxes
            MAKE_WIN32_ALIAS(CreateDialogIndirectParam)
            MAKE_WIN32_ALIAS(CreateDialogParam)
            MAKE_WIN32_ALIAS(DefDlgProc)
            MAKE_WIN32_ALIAS(DialogBoxIndirectParam)
            MAKE_WIN32_ALIAS(DialogBoxParam)
            MAKE_WIN32_ALIAS(GetDlgItemText)
            MAKE_WIN32_ALIAS(IsDialogMessage)
            MAKE_WIN32_ALIAS(MessageBox)
            MAKE_WIN32_ALIAS(MessageBoxEx)
            MAKE_WIN32_ALIAS(MessageBoxIndirect)
            MAKE_WIN32_ALIAS(SendDlgItemMessage)
            MAKE_WIN32_ALIAS(SetDlgItemText)
         #pragma endregion
         #pragma region Keyboard and Mouse Input
            MAKE_WIN32_ALIAS(GetKeyNameText)
            MAKE_WIN32_ALIAS(GetKeyboardLayoutName)
            MAKE_WIN32_ALIAS(GetRawInputDeviceInfo)
            MAKE_WIN32_ALIAS(LoadKeyboardLayout)
            MAKE_WIN32_ALIAS(MapVirtualKey)
            MAKE_WIN32_ALIAS(MapVirtualKeyEx)
            MAKE_WIN32_ALIAS(VkKeyScan)
            MAKE_WIN32_ALIAS(VkKeyScanEx)
         #pragma endregion
         #pragma region Menus and Other Resources
            MAKE_WIN32_ALIAS(AppendMenu)
            MAKE_WIN32_ALIAS(CharLower)
            MAKE_WIN32_ALIAS(CharLowerBuff)
            MAKE_WIN32_ALIAS(CharNext)
            MAKE_WIN32_ALIAS_A_ONLY(CharNextEx)
            MAKE_WIN32_ALIAS(CharPrev)
            MAKE_WIN32_ALIAS_A_ONLY(CharPrevEx)
            MAKE_WIN32_ALIAS(CharToOem)
            MAKE_WIN32_ALIAS(CharToOemBuff)
            MAKE_WIN32_ALIAS(CharUpper)
            MAKE_WIN32_ALIAS(CharUpperBuff)
            MAKE_WIN32_ALIAS(CopyAcceleratorTable)
            MAKE_WIN32_ALIAS(CreateAcceleratorTable)
            MAKE_WIN32_ALIAS(GetIconInfoEx)
            MAKE_WIN32_ALIAS(GetMenuItemInfo)
            MAKE_WIN32_ALIAS(GetMenuString)
            MAKE_WIN32_ALIAS(InsertMenu)
            MAKE_WIN32_ALIAS(InsertMenuItem)
            MAKE_WIN32_ALIAS(IsCharAlpha)
            MAKE_WIN32_ALIAS(IsCharAlphaNumeric)
            MAKE_WIN32_ALIAS(IsCharLower)
            MAKE_WIN32_ALIAS(IsCharUpper)
            MAKE_WIN32_ALIAS(LoadAccelerators)
            MAKE_WIN32_ALIAS(LoadCursor)
            MAKE_WIN32_ALIAS(LoadCursorFromFile)
            MAKE_WIN32_ALIAS(LoadIcon)
            MAKE_WIN32_ALIAS(LoadImage)
            MAKE_WIN32_ALIAS(LoadMenu)
            MAKE_WIN32_ALIAS(LoadMenuIndirect)
            MAKE_WIN32_ALIAS(LoadString)
            MAKE_WIN32_ALIAS(ModifyMenu)
            MAKE_WIN32_ALIAS(OemToChar)
            MAKE_WIN32_ALIAS(OemToCharBuff)
            MAKE_WIN32_ALIAS(PrivateExtractIcons)
            MAKE_WIN32_ALIAS(SetMenuItemInfo)
            MAKE_WIN32_ALIAS(TranslateAccelerator)
         #pragma endregion
         #pragma region System Services
            MAKE_WIN32_ALIAS(RegisterDeviceNotification)
         #pragma endregion
         #pragma region The Windows Shell
            MAKE_WIN32_ALIAS(WinHelp)
         #pragma endregion
         #pragma region Window Stations and Desktops
            MAKE_WIN32_ALIAS(CreateDesktop)
            MAKE_WIN32_ALIAS(CreateDesktopEx)
            MAKE_WIN32_ALIAS(CreateWindowStation)
            MAKE_WIN32_ALIAS(EnumDesktops)
            MAKE_WIN32_ALIAS(EnumWindowStations)
            MAKE_WIN32_ALIAS(GetUserObjectInformation)
            MAKE_WIN32_ALIAS(OpenDesktop)
            MAKE_WIN32_ALIAS(OpenWindowStation)
            MAKE_WIN32_ALIAS(SetUserObjectInformation)
         #pragma endregion
         #pragma region Windows and Messages
            MAKE_WIN32_ALIAS(BroadcastSystemMessage)
            MAKE_WIN32_ALIAS(BroadcastSystemMessageEx)
            MAKE_WIN32_ALIAS(CallMsgFilter)
            MAKE_WIN32_ALIAS(CallWindowProc)
            MAKE_WIN32_ALIAS(CreateMDIWindow)
            MAKE_WIN32_ALIAS(CreateWindowEx)
            MAKE_WIN32_ALIAS(DefFrameProc)
            MAKE_WIN32_ALIAS(DefMDIChildProc)
            MAKE_WIN32_ALIAS(DefWindowProc)
            MAKE_WIN32_ALIAS(DispatchMessage)
            MAKE_WIN32_ALIAS(EnumProps)
            MAKE_WIN32_ALIAS(EnumPropsEx)
            MAKE_WIN32_ALIAS(FindWindow)
            MAKE_WIN32_ALIAS(FindWindowEx)
            MAKE_WIN32_ALIAS(GetAltTabInfo)
            MAKE_WIN32_ALIAS(GetClassInfo)
            MAKE_WIN32_ALIAS(GetClassInfoEx)
            MAKE_WIN32_ALIAS(GetClassLong)
            MAKE_WIN32_ALIAS(GetClassLongPtr)
            MAKE_WIN32_ALIAS(GetClassName)
            MAKE_WIN32_ALIAS(GetMessage)
            MAKE_WIN32_ALIAS(GetProp)
            MAKE_WIN32_ALIAS(GetWindowLong)
            MAKE_WIN32_ALIAS(GetWindowLongPtr)
            MAKE_WIN32_ALIAS(GetWindowModuleFileName)
            MAKE_WIN32_ALIAS(GetWindowText)
            MAKE_WIN32_ALIAS(GetWindowTextLength)
            MAKE_WIN32_ALIAS(PeekMessage)
            MAKE_WIN32_ALIAS(PostMessage)
            MAKE_WIN32_ALIAS(PostThreadMessage)
            MAKE_WIN32_ALIAS(RealGetWindowClass)
            MAKE_WIN32_ALIAS(RegisterClass)
            MAKE_WIN32_ALIAS(RegisterClassEx)
            MAKE_WIN32_ALIAS(RegisterWindowMessage)
            MAKE_WIN32_ALIAS(RemoveProp)
            MAKE_WIN32_ALIAS(SendMessage)
            MAKE_WIN32_ALIAS(SendMessageCallback)
            MAKE_WIN32_ALIAS(SendMessageTimeout)
            MAKE_WIN32_ALIAS(SendNotifyMessage)
            MAKE_WIN32_ALIAS(SetClassLong)
            MAKE_WIN32_ALIAS(SetClassLongPtr)
            MAKE_WIN32_ALIAS(SetProp)
            MAKE_WIN32_ALIAS(SetWindowLong)
            MAKE_WIN32_ALIAS(SetWindowLongPtr)
            MAKE_WIN32_ALIAS(SetWindowText)
            MAKE_WIN32_ALIAS(SetWindowsHookEx)
            MAKE_WIN32_ALIAS(SystemParametersInfo)
            MAKE_WIN32_ALIAS(UnregisterClass)
         #pragma endregion
         #pragma region Windows Controls
            MAKE_WIN32_ALIAS(DlgDirList)
            MAKE_WIN32_ALIAS(DlgDirListComboBox)
            MAKE_WIN32_ALIAS(DlgDirSelectComboBoxEx)
            MAKE_WIN32_ALIAS(DlgDirSelectEx)
         #pragma endregion
         #pragma region Windows GDI
            MAKE_WIN32_ALIAS(ChangeDisplaySettings)
            MAKE_WIN32_ALIAS(ChangeDisplaySettingsEx)
            MAKE_WIN32_ALIAS(DrawState)
            MAKE_WIN32_ALIAS(DrawText)
            MAKE_WIN32_ALIAS(DrawTextEx)
            MAKE_WIN32_ALIAS(EnumDisplayDevices)
            MAKE_WIN32_ALIAS(EnumDisplaySettings)
            MAKE_WIN32_ALIAS(EnumDisplaySettingsEx)
            MAKE_WIN32_ALIAS(GetMonitorInfo)
            MAKE_WIN32_ALIAS(GetTabbedTextExtent)
            MAKE_WIN32_ALIAS(GrayString)
            MAKE_WIN32_ALIAS(LoadBitmap)
            MAKE_WIN32_ALIAS(TabbedTextOut)
         #pragma endregion
      #pragma endregion
      #ifdef COBB_WIN32_ENABLE_VERSION
         #pragma region Winver.h
            #ifndef VER_H
               #error Windows.h should've included this header. What happened?
            #endif
            #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
               MAKE_WIN32_ALIAS(GetFileVersionInfo)
               MAKE_WIN32_ALIAS(GetFileVersionInfoEx)
               MAKE_WIN32_ALIAS(GetFileVersionInfoSize)
               MAKE_WIN32_ALIAS(GetFileVersionInfoSizeEx)
               MAKE_WIN32_ALIAS(VerFindFile)
            #endif
            #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
               MAKE_WIN32_ALIAS(VerInstallFile)
            #endif
            #if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
               MAKE_WIN32_ALIAS(VerLanguageName)
               MAKE_WIN32_ALIAS(VerQueryValue)
            #endif
         #pragma endregion
      #endif

      #undef MAKE_WIN32_ALIAS
      #undef MAKE_WIN32_ALIAS_A_ONLY
      #pragma pop_macro("MAKE_WIN32_ALIAS")
      #pragma pop_macro("MAKE_WIN32_ALIAS_A_ONLY")

      #ifdef COBB_WIN32_IN_NAMESPACE
      }
      #endif
      
   #pragma endregion
#endif