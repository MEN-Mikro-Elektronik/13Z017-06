<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>z17_doc.c</name>
    <path>/home/dpfeuffer/WORK/13z017-06/DRIVERS/MDIS_LL/Z017/DRIVER/COM/</path>
    <filename>z17__doc_8c</filename>
  </compound>
  <compound kind="file">
    <name>z17_drv.c</name>
    <path>/home/dpfeuffer/WORK/13z017-06/DRIVERS/MDIS_LL/Z017/DRIVER/COM/</path>
    <filename>z17__drv_8c</filename>
    <class kind="struct">LL_HANDLE</class>
    <member kind="define">
      <type>#define</type>
      <name>_NO_LL_HANDLE</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a7ed5e038d7cded6c7eca1ca9e38710be</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CH_NUMBER</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a8a005ca68f582be09b6df19202d193f4</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>USE_IRQ</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a27c634d2ee9b16476067697106830240</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ADDRSPACE_COUNT</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a9a59fa83d3160499460a6788485ac8ec</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ADDRSPACE_SIZE</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a4f07921d496dd6047ca73837f7b97905</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DBG_MYLEVEL</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a8697e24328845380f3e287a9987cbd96</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DBH</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>afb81e493b2996d3018cffb7a6946b78c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>OSH</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a2b0f11014eb4a12ccb73b0828d297bbf</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TOG_TIME_DEFAULT</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a36919f0934b05601e0337f2ded12f61e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TOG_TIME_MIN</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a27c72d55dfe9fa12c589f1105b1cfd17</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TOG_TIME_MAX</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a66bb64eb668dc7c4779bfdf685ceca1a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TOG_CYCLIC</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a6c8af06ca7062e281c57c7b233a767e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RESET_DEFAULT</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a849d9305357c3f914020fa65b0522b3c</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RESET_OFF</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>afa9ce51b015785118fcd59f631a3a5ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_Init</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>aa47456858f4feff1ce123f08245497dc</anchor>
      <arglist>(DESC_SPEC *descP, OSS_HANDLE *osHdl, MACCESS *ma, OSS_SEM_HANDLE *devSemHdl, OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_Exit</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a6fd3b67f2690a64ebdb55cde827c88e3</anchor>
      <arglist>(LL_HANDLE **llHdlP)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_Read</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>accffa2bf35599ca8d55beb84927036f0</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 ch, int32 *valueP)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_Write</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a4df445d09d7e7b04b03fa38a5ef9a358</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 ch, int32 value)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_SetStat</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a4d2755e2b05c996a9d03ec0dbefd654a</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 code, int32 ch, INT32_OR_64 value32_or_64)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_GetStat</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a3eb627d23bf92b080243c988a58a8c94</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 code, int32 ch, INT32_OR_64 *value32_or_64P)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_BlockRead</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a3c909ebf901146afeff99b2ef8c72838</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size, int32 *nbrRdBytesP)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_BlockWrite</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a729589ea6f0de88c944b72f9e8a073eb</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size, int32 *nbrWrBytesP)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_Irq</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>a8cf8b46e2555297d360a663f6be137fa</anchor>
      <arglist>(LL_HANDLE *llHdl)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Z17_Info</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>acfedafe4726bbf92c1a325c6ece7a158</anchor>
      <arglist>(int32 infoType,...)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static char *</type>
      <name>Ident</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>ae89983c538f30479c0b65ba4a22a32e0</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32</type>
      <name>Cleanup</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>acdf5f419c6b0a18c5de79e4a9c8bf3ad</anchor>
      <arglist>(LL_HANDLE *llHdl, int32 retCode)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>arwenReset</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>aa9959369b1e08d4007e6157a1bf97df2</anchor>
      <arglist>(LL_HANDLE *llHdl)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>AlarmHandler</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>aabeac115dd99e4fdfa754e1d863058dc</anchor>
      <arglist>(void *arg)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>__Z17_GetEntry</name>
      <anchorfile>z17__drv_8c.html</anchorfile>
      <anchor>aea64fa98e94897d71418835d8ee05054</anchor>
      <arglist>(LL_ENTRY *drvP)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>LL_HANDLE</name>
    <filename>structLL__HANDLE.html</filename>
    <member kind="variable">
      <type>int32</type>
      <name>memAlloc</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a3f1aa2b73cb056cf8b16943fe4a036c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OSS_HANDLE *</type>
      <name>osHdl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>acea42045f1b7df8b9a384bd4d383877d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OSS_IRQ_HANDLE *</type>
      <name>irqHdl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a1022262bf149bd2c2b8e94416435033c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>DESC_HANDLE *</type>
      <name>descHdl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a355f08452b33262cc1195fa301416da5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MACCESS</type>
      <name>ma</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>afdf907a01b53fdb8b192543288102c06</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MDIS_IDENT_FUNCT_TBL</type>
      <name>idFuncTbl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a47d151b3bf404c03d0bfc080b2184042</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>dbgLevel</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a413af872055c08749a2b451d3c0ceda9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>DBG_HANDLE *</type>
      <name>dbgHdl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a308eab31e057c8002afa3cc63e296f2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OSS_SIG_HANDLE *</type>
      <name>portChangeSig</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>ad5cf5f470c378bc85384da16e02893a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>irqCount</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>ace8d8e9be894922209b95a7ca8d7bde7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>lastReq</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a7b296a6ad5dd0a1e60554db6d88642e8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>irqTest</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>af3c8e36d239554b2c9ba528f8992cded</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32</type>
      <name>outBit</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a8ceaf810973f26b07e830bd788ec3a5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>outLast</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a6e14a3b7874a659afa83c090a83a9e03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>irqs2fire</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a2992fa0ff6e0c6629062d3731c6ee09b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>startTick</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a3481572a5c4740e1f8059b972686a1e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>stopTick</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a2d8bddecf259174b86e21212021df3dc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OSS_ALARM_HANDLE *</type>
      <name>alarmHdl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>ac1b7c71df67d06741fce4423a4cf6ae2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>OSS_SEM_HANDLE *</type>
      <name>devSemHdl</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>aedf84a8c0c2797bc6004dfe484ec8ba0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>togHigh</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a6486996fb889acbeb2e4350910dc1217</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>togtimeHigh</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a0585f630a3f100f34d8fc3f7fa1d51a0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>togtimeLow</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>af0a4841e98dd83b7e2ea1d1b0ce5ec28</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>togBitMask</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a203dabc123b91689f603c5888f1cd1e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>togCount</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>acc51beaa2e4ec7d165908514971f47cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u_int32</type>
      <name>arwenResetOff</name>
      <anchorfile>structLL__HANDLE.html</anchorfile>
      <anchor>a19e0281a2e4c80b08cc33213512d721e</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>dummy</name>
    <title></title>
    <filename>dummy</filename>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title></title>
    <filename>index</filename>
    <docanchor file="index" title="Variants">Variants</docanchor>
    <docanchor file="index" title="Functional Description">FuncDesc</docanchor>
    <docanchor file="index" title="General">General</docanchor>
    <docanchor file="index" title="Reading and writing">readwrite</docanchor>
    <docanchor file="index" title="Interrupt handling and signals">interrupts</docanchor>
    <docanchor file="index" title="Open Drain feature">opendrain</docanchor>
    <docanchor file="index" title="Debouncing">debounce</docanchor>
    <docanchor file="index" title="Default values">default</docanchor>
    <docanchor file="index" title="SA15 issues">sa15</docanchor>
    <docanchor file="index" title="Supported API Functions">api_functions</docanchor>
    <docanchor file="index" title="Descriptor Entries">descriptor_entries</docanchor>
    <docanchor file="index" title="Z17 specific Getstat/Setstat codes">codes</docanchor>
    <docanchor file="index" title="Overview of all Documents">Documents</docanchor>
    <docanchor file="index" title="Simple example for using the driver">z17_simp</docanchor>
    <docanchor file="index" title="Tool to access the 16Z034/16Z037 I/Os">z17_io</docanchor>
    <docanchor file="index" title="Tool to access the 16Z127 I/Os">z127_io</docanchor>
    <docanchor file="index" title="Minimum descriptor">z17_min</docanchor>
    <docanchor file="index" title="Maximum descriptor">z17_max</docanchor>
  </compound>
</tagfile>
