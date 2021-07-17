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
<link rel="stylesheet" href="faq.css" type="text/css"/>
<title>Cracklock FAQ</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
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
<body>
<h1 id="page-title">Cracklock FAQ</h1>

<h1>Table of content</h1>
<div class="toolbox">
<a href="javascript:toggle_all('block')">Expand all</a>
<a href="javascript:toggle_all('none')">Collpase</a>
</div>
<xsl:call-template name="contents"/>

<h1>Author</h1>
<xsl:call-template name="credit-list"/>

<xsl:apply-templates/>

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
	<a><xsl:attribute name="name"><xsl:number count="section" level="multiple"/></xsl:attribute></a>
	<h1><xsl:number count="section" format="1" level="multiple"/>. <xsl:value-of select="@title"/></h1>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="section/section">
	<a><xsl:attribute name="name"><xsl:number count="section" level="multiple"/></xsl:attribute></a>
	<h2><xsl:number count="section" format="1" level="multiple"/>. <xsl:value-of select="@title"/></h2>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="section/section/section">
	<a><xsl:attribute name="name"><xsl:number count="section" level="multiple"/></xsl:attribute></a>
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
<table width="100%">
<tr>
<td width="20%" valign="top">
<p>
<a title="title"><xsl:attribute name="href">javascript:toggle_answer('ans<xsl:number count="section|qa" format="1" level="multiple"/>');</xsl:attribute>[+]</a>
<xsl:text> </xsl:text>
<b>Question</b>
</p>
<xsl:call-template name="author"/>
</td>
<td width="80%" valign="top"><xsl:call-template name="article"/></td>
</tr>
</table>
</div>
</xsl:template>

<xsl:template match="answer">
<div class="answer">
<table width="100%">
<tr>
<td width="20%" valign="top"><p><b>Answer</b></p><xsl:call-template name="author"/></td>
<td width="80%" class="answer" valign="top"><xsl:call-template name="article"/></td>
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