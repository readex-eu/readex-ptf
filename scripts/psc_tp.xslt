<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:psc="http://www.lrr.in.tum.de/Periscope"
                xmlns="http://www.w3.org/TR/xhtml1/strict">

  <!-- Purpose: Update a SIR files automatically generated with the psc_instrument with a tuning action -->
  <!-- Project: Autotune -->
  <!-- Date: September 2014 -->
  <!-- Author: L. Morin -->

  <xsl:param name="fileid" select="'1'" />
  <xsl:param name="position" select="''" />
  <xsl:param name="action" select="'TUNE_CODE'" />
  <xsl:param name="variants" select="'3'" />

  <!-- Duplicate the tree and re-apply templates -->
  <xsl:template match="*">
    <xsl:copy><xsl:copy-of select="@*"/><xsl:apply-templates select="node()"/></xsl:copy>
  </xsl:template>

  <!-- Duplicate Comments -->
  <xsl:template match="comment()">
    <xsl:comment>
      <xsl:value-of select="." />
    </xsl:comment>
  </xsl:template>

  <!-- Match code regions, duplicate the position, and append  -->
  <xsl:template match="psc:codeRegion|codeRegion">
    <xsl:copy>
      <xsl:copy-of select="@*|text()[1]"/>
      <xsl:for-each select="position">
        <xsl:copy><xsl:copy-of select="@*|node()"/></xsl:copy>
      </xsl:for-each>
      <xsl:if test="contains(@id,concat($fileid,'-',number($position)+1))">
        <xsl:comment>Tuning action applied</xsl:comment>
        <xsl:text>
        </xsl:text>
        <plugin pluginId="USER" xmlns=""><xsl:text>
        </xsl:text>
        <selector tuningActionType="VAR">
          <xsl:attribute name="tuningActionName"><xsl:value-of select="$action"/></xsl:attribute>
          <xsl:attribute name="numberOfVariants"><xsl:value-of select="$variants"/></xsl:attribute>
        </selector>
        </plugin>
      </xsl:if>
      <xsl:copy-of select="text()[2]"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
