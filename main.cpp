#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>

#define UFG_PAD_INSERT(x, y) x ## y
#define UFG_PAD_DEFINE(x, y) UFG_PAD_INSERT(x, y)
#define UFG_PAD(size) char UFG_PAD_DEFINE(padding_, __LINE__)[size]

int main()
{
    struct HavokAnimData_t
    {
        uint32_t m_Magic[2];
        int m_UserTag;
        int m_FileVersion;
        char m_LayoutRules[4];
        int m_NumSections;
        int m_ContentsSectionIndex;
        int m_ContentsSectionOffset;
        int m_ContentsClassNameSectionIndex;
        int m_ContentsClassNameSectionOffset;
        char m_ContentsVersion[16];
        int m_Flags;
        int m_Pad[1];

        bool IsMagicValid()
        {
            return (m_Magic[0] == 0x57E0E057);
        }
    };

    struct AnimationResource_t
    {
        UFG_PAD(0x44);

        char m_Name[28];

        UFG_PAD(0x8);

        uint32_t m_Size;

        UFG_PAD(0x14);

        HavokAnimData_t m_HavokAnimData;
    };

    WIN32_FIND_DATAA m_FindData = { 0 };
    HANDLE m_FindHandle = FindFirstFileA("Anims\\*.bin", &m_FindData);
    if (m_FindHandle != INVALID_HANDLE_VALUE)
    {
        while (1)
        {
            std::string m_AnimationGroupName = m_FindData.cFileName;
            m_AnimationGroupName.erase(m_AnimationGroupName.size() - 4, 4); // Remove '.bin'

            std::string m_AnimationFilePath = "Anims\\" + std::string(m_FindData.cFileName);
            FILE* m_AnimFile = fopen(&m_AnimationFilePath[0], "rb");
            if (m_AnimFile)
            {
                std::string m_AnimationFolder = "Anims\\" + m_AnimationGroupName;
                CreateDirectoryA(&m_AnimationFolder[0], 0);

                printf("[ ~ ] %s:\n", &m_AnimationGroupName[0]);

                fseek(m_AnimFile, 0L, SEEK_END);

                long m_AnimFileSize = ftell(m_AnimFile);

                fseek(m_AnimFile, 0L, SEEK_SET);

                uint8_t* m_AnimFileData = new uint8_t[m_AnimFileSize];
                long m_AnimFileOffset = 0;

                if (fread(m_AnimFileData, m_AnimFileSize, sizeof(uint8_t), m_AnimFile))
                {
                    while (m_AnimFileSize > m_AnimFileOffset)
                    {
                        AnimationResource_t* m_AnimationResource = reinterpret_cast<AnimationResource_t*>(&m_AnimFileData[m_AnimFileOffset]);
                        if (!m_AnimationResource->m_HavokAnimData.IsMagicValid())
                            break;

                        std::string m_AnimationFilePath = m_AnimationFolder + "\\" + m_AnimationResource->m_Name + ".hkx";
                        FILE* m_AnimationFile = fopen(&m_AnimationFilePath[0], "wb");
                        if (m_AnimationFile)
                        {
                            printf("[ + ] %s\n", m_AnimationResource->m_Name);

                            fwrite(&m_AnimationResource->m_HavokAnimData, m_AnimationResource->m_Size, sizeof(uint8_t), m_AnimationFile);
                            fclose(m_AnimationFile);
                        }

                        m_AnimFileOffset += (sizeof(AnimationResource_t) - sizeof(HavokAnimData_t)) + m_AnimationResource->m_Size;
                    }
                }

                delete[] m_AnimFileData;
                fclose(m_AnimFile);

                printf("\n");
            }

            if (!FindNextFileA(m_FindHandle, &m_FindData))
                break;
        }

        FindClose(m_FindHandle);
    }

    return 0;
}
