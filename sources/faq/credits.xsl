<?xml version="1.0" encoding="koi8-r"?>

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output
   method="text"
   indent="yes"
   encoding="koi8-r"
/>

<xsl:template match="faq">
	<xsl:for-each select="//answer">
		<xsl:if test="@author">
			<author>
			<xsl:value-of select="@author"/>
			</author>
			<xsl:text>
</xsl:text>		
		</xsl:if>
	</xsl:for-each>
	<xsl:for-each select="//question">
		<xsl:if test="@author">
			<xsl:value-of select="@author"/>
			<xsl:text>
</xsl:text>			
		</xsl:if>
	</xsl:for-each>
</xsl:template>

</xsl:stylesheet>