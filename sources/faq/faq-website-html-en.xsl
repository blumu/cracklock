<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml">

<xsl:output
   method="html"
   indent="yes"
   encoding="utf-8"
   doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN" 
   doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
/>


<xsl:template match="faq">
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
  <script src="http://www.google-analytics.com/urchin.js" type="text/javascript">
  </script>
  <script type="text/javascript">
  _uacct = "UA-77730-1";
  urchinTracker();
  </script>
  <link href="../../perso.css" rel="stylesheet" type="text/css" />
  <link rel="stylesheet" href="webfaq.css" type="text/css"/>

<title>Cracklock FAQ</title>
<script language="javascript" type="text/javascript">
<xsl:comment>
function toggle_answer(id)
{
	var el = document.getElementById(id);
	if (el.style.display == "block") {
		el.style.display = "none";
	} else {
		el.style.display = "block";
	}
}

function toggle_all(st)
{
	var el = document.getElementById('ans1.1.1');
	el.style.display = st;
}
//</xsl:comment>
</script>
</head>

<!-- For inclusion in my homepage -->
<body id="threecolumn" background="images/fond.jpg">
<div id="container">
	<xsl:text disable-output-escaping="yes">&lt;?php </xsl:text>
	<xsl:text disable-output-escaping="yes">
			$cursection="software";
			$cursubsection="Cracklock";
			include("../../common/arround.html");
	</xsl:text>
	<xsl:text disable-output-escaping="yes">?></xsl:text>

	<div id="main-content">
<h1 id="page-title">
<img src ="cracklock.png" alt="Cracklock" width="60" height="60" border="0" align="middle" />
Cracklock FAQ</h1>

<!-- ... for inclusion in my homepage. -->

<h1>Table of content</h1>
<div class="toolbox">
<a href="javascript:toggle_all('block')">Expand all</a>
<a href="javascript:toggle_all('none')">Collpase</a>
</div>
<xsl:call-template name="contents"/>

<!--
<h1>Author</h1>
<xsl:call-template name="credit-list"/>
-->

<xsl:apply-templates/>

<!-- For inclusion in my homepage -->
</div>
	<xsl:text disable-output-escaping="yes">&lt;?php </xsl:text>
	<xsl:text disable-output-escaping="yes">
			include("../../common/footer.html");
	</xsl:text>
	<xsl:text disable-output-escaping="yes">?></xsl:text>
</div>
<!-- ... for inclusion in my homepage. -->

</body>
</html>
</xsl:template>

<xsl:template name="contents">
	<xsl:for-each select="section">
		<xsl:call-template name="section-contents"/>
	</xsl:for-each>
</xsl:template>

<xsl:template name="section-contents">
	<p>
	<a><xsl:attribute name="href">#<xsl:number count="section" level="multiple"/></xsl:attribute><xsl:number count="section" level="multiple"/>. <xsl:value-of select="@title"/></a>
	</p>
	<xsl:if test="section">
		<blockquote>
		<xsl:for-each select="section">
			<xsl:call-template name="section-contents"/>
		</xsl:for-each>
		</blockquote>
	</xsl:if>
</xsl:template>

<xsl:template name="credit-list">
	<ul>
	<xsl:for-each select="document('credits.xml')/authors/author">
		<li><xsl:value-of select="."/></li>
	</xsl:for-each>
	</ul>
</xsl:template>

<xsl:template match="section">
	<a class="section"><xsl:attribute name="name"><xsl:number count="section" level="multiple"/></xsl:attribute></a>
	<h1><xsl:number count="section" format="1" level="multiple"/>. <xsl:value-of select="@title"/></h1>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="section/section">
	<a class="section"><xsl:attribute name="name"><xsl:number count="section" level="multiple"/></xsl:attribute></a>
	<h2><xsl:number count="section" format="1" level="multiple"/>. <xsl:value-of select="@title"/></h2>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="section/section/section">
	<a class="section"><xsl:attribute name="name"><xsl:number count="section" level="multiple"/></xsl:attribute></a>
	<h3><xsl:number count="section" format="1" level="multiple"/>. <xsl:value-of select="@title"/></h3>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="qa">
<div class="qa">
<xsl:apply-templates select="question"/>
<div>
<xsl:attribute name="id">ans<xsl:number count="section|qa" format="1" level="multiple"/></xsl:attribute>
<xsl:attribute name="style">display: none</xsl:attribute>
<xsl:apply-templates select="answer"/>
<xsl:apply-templates select="link"/>
</div>
</div>
</xsl:template>

<xsl:template match="question">
<div class="question">
<table class="questiontable" >
<tr class="questionrow">
<xsl:attribute name="onclick">javascript:toggle_answer('ans<xsl:number count="section|qa" format="1" level="multiple"/>');</xsl:attribute>
<td class="firstcol" valign="top">
<a class="drilldown">[+]</a>
<xsl:text> </xsl:text>
<xsl:call-template name="author"/>
</td>
<td class="questioncell" valign="top"><xsl:call-template name="article"/></td>
</tr>
</table>
</div>
</xsl:template>

<xsl:template match="answer">
<div class="answer">
<table class="answertable" >
<tr class="answerrow">
<td class="firstcol" valign="top"><xsl:call-template name="author"/></td>
<td class="answercell" valign="top"><xsl:call-template name="article"/></td>
</tr>
</table>
</div>
</xsl:template>

<xsl:template match="link">
<div class="qa-link">
<a><xsl:attribute name="href"><xsl:value-of select="."/></xsl:attribute>qalink...</a>
</div>
</xsl:template>

<xsl:template name="author">
<xsl:if test="@author"><p>[ <xsl:value-of select="@author"/> ]</p></xsl:if>
</xsl:template>

<xsl:template name="article">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="pre">
<pre><xsl:apply-templates/></pre>
</xsl:template>

<xsl:template match="p">
<p><xsl:apply-templates/></p>
</xsl:template>

<xsl:template match="ul">
<ul><xsl:apply-templates/></ul>
</xsl:template>

<xsl:template match="ol">
<ol><xsl:apply-templates/></ol>
</xsl:template>

<xsl:template match="li">
<li><xsl:apply-templates/></li>
</xsl:template>

<xsl:template match="b">
<b><xsl:apply-templates/></b>
</xsl:template>

<xsl:template match="br">
<br/>
</xsl:template>

</xsl:stylesheet>